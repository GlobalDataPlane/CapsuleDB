#include <openssl/sha.h>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <sstream>
#include <iostream>
#include <fstream>
#include <vector>
#include "capsuleBlock.hh"
#include "fakeCapsule.hh"
#include "src/core/capsuleBlock.pb.h"
#include "src/uuid_v4/uuid_v4.h"

void sha256_string(const char *string, char outputBuffer[65])
{
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, string, strlen(string));
    SHA256_Final(hash, &sha256);
    int i = 0;
    for(i = 0; i < SHA256_DIGEST_LENGTH; i++)
    {
        sprintf(outputBuffer + (i * 2), "%02x", hash[i]);
    }
    outputBuffer[64] = 0;
}

std::string putCapsuleBlockBoost(CapsuleBlock inputBlock) {
    // Serialize Block
    std::stringstream toBeHashed;
    boost::archive::text_oarchive oa1(toBeHashed);
    oa1 << inputBlock;
    std::string s = toBeHashed.str();
    // std::cout << "putCapsuleBlock: toBeHashed=" << s << "\n";

    // Hash bytestream
    char blockHash[65];
    sha256_string(s.data(), blockHash);
    // std::cout << "putCapsuleBlock: blockHash=" << blockHash << "\n";

    // Serialize and store block
    std::ofstream storedBlockFile(blockHash);
    boost::archive::text_oarchive oa2(storedBlockFile);
    oa2 << inputBlock;

    // Return Hash
    return blockHash;
}

CapsuleBlock getCapsuleBlockBoost(std::string inputHash) {
    CapsuleBlock recoveredBlock;
    // Retrieve and deserialize block
    std::ifstream storedBlock(inputHash);
    boost::archive::text_iarchive ia(storedBlock);
    ia >> recoveredBlock;

    // * Re-serialize and check hash *
    // ** Serialize Block **
    std::stringstream toBeHashed;
    boost::archive::text_oarchive oa1(toBeHashed);
    oa1 << recoveredBlock;
    std::string s = toBeHashed.str();
    // std::cout << "getCapsuleBlock: toBeHashed=" << s << "\n";
    // ** Hash bytestream ** 
    char blockHash[65];
    sha256_string(s.data(), blockHash);
    // std::cout << "getCapsuleBlock: blockHash=" << blockHash << "\n";
    // ** Verify hash **
    if (blockHash != inputHash) {
        std::cout << "inputHash=" << inputHash << "\n";
        throw std::invalid_argument("inputHash not found");
    }
    // Return to user
    return recoveredBlock;
}

/*
    Input = CapsuleBlock
    Value = serialize(CapsuleBlock)
    Key = hash(Value)
        Store Value into a file named Key
    Output = Key
*/
std::string putCapsuleBlock(CapsuleBlock inputBlock)
{
    // Serialize Block
    capsuleDBSerialization::CapsuleBlock protobufBlock;

    protobufBlock.set_level(inputBlock.getLevel());
    protobufBlock.set_startkey(inputBlock.getMinKey());
    protobufBlock.set_endkey(inputBlock.getMaxKey());

    UUIDv4::UUIDGenerator<std::mt19937_64> uuidGenerator;
    UUIDv4::UUID uuid = uuidGenerator.getUUID();

    protobufBlock.set_uuid(uuid.bytes());
    
    std::vector<kvs_payload> pairs = inputBlock.getKVPairs();
    for (int i = 0; i < pairs.size(); i++) {
        capsuleDBSerialization::kvs_payload* kvPair_proto = protobufBlock.add_kvpairs();
        kvPair_proto->set_key(pairs[i].key);
        kvPair_proto->set_value(pairs[i].value);
        kvPair_proto->set_txn_timestamp(pairs[i].txn_timestamp);
        kvPair_proto->set_txn_msgtype(pairs[i].txn_msgType);
    }

    // std::cout << "Level: " << protobufBlock.level() << std::endl;
    // std::cout << "Start key: " << protobufBlock.startkey() << std::endl;
    // std::cout << "End key: " << protobufBlock.endkey() << std::endl;
    // for (int i = 0; i < pairs.size(); i++) {
    //     capsuleDBSerialization::kvs_payload kvPair_proto = protobufBlock.kvpairs(i);
    //     std::cout << "Key: " << kvPair_proto.key() << " Value: " << kvPair_proto.value() << " Timestamp: " << kvPair_proto.txn_timestamp() << " Msg: " << kvPair_proto.txn_msgtype() << std::endl;
    // }
    
    std::string serializedBlock;
    if (!protobufBlock.SerializeToString(&serializedBlock)) {
        std::cerr << "Failed to serialize CapsuleBlock" << std::endl;
        return "";
    }
    // std::cout << "putCapsuleBlock: toBeHashed=" << serializedBlock << "\n";

    // Hash bytestream
    char blockHash[65];
    sha256_string(serializedBlock.data(), blockHash);
    std::cout << "putCapsuleBlock: blockHash=" << blockHash << "\n";

    // Store serialized block in file
    std::ofstream storedBlockFile;
    storedBlockFile.open(blockHash);
    storedBlockFile << serializedBlock;
    storedBlockFile.close();

    // google::protobuf::ShutdownProtobufLibrary();

    // return Hash
    return blockHash;
}

CapsuleBlock getCapsuleBlock(std::string inputHash)
{
    capsuleDBSerialization::CapsuleBlock recoveredBlock;
    // Retrieve and deserialize block
    std::ifstream storedBlock;
    storedBlock.open(inputHash);
    if (!recoveredBlock.ParseFromIstream(&storedBlock)) {
        std::cerr << "Failed to parse CapsuleBlock" << std::endl;
        return NULL;
    }
    storedBlock.close();

    // * Re-serialize and check hash *
    // ** Serialize Block **
    std::string serializedBlock;
    if (!recoveredBlock.SerializeToString(&serializedBlock)) {
        std::cerr << "Failed to serialize CapsuleBlock" << std::endl;
        return NULL;
    }
    // std::cout << "getCapsuleBlock: toBeHashed=" << serializedBlock << "\n";
    // ** Hash bytestream **
    char blockHash[65];
    sha256_string(serializedBlock.data(), blockHash);
    // std::cout << "getCapsuleBlock: blockHash=" << blockHash << "\n";
    // ** Verify hash **
    if (blockHash != inputHash) {
        std::cout << "inputHash=" << inputHash << "\n";
        throw std::invalid_argument("inputHash not found");
    }

    // Convert recoveredBlock(proto) to actual CapsuleBlock
    CapsuleBlock actualBlock(recoveredBlock.level());
    actualBlock.setMinKey(recoveredBlock.startkey());
    actualBlock.setMaxKey(recoveredBlock.endkey());

    for (int i = 0; i <  recoveredBlock.kvpairs_size(); i++) {
      const capsuleDBSerialization::kvs_payload& kvPair_proto = recoveredBlock.kvpairs(i);
      kvs_payload kvPair = {kvPair_proto.key(), kvPair_proto.value(),
                            kvPair_proto.txn_timestamp(),
                            kvPair_proto.txn_msgtype()};
      actualBlock.addKVPair(kvPair);

        // std::cout << "Key from protobuf: " << kvPair_proto.key() << " Key from block: " << kvPair.key << std::endl;
        // std::cout << "Value from protobuf: " << kvPair_proto.value() << " Value from block: " << kvPair.value << std::endl;
        // std::cout << "Timestamp from protobuf: " << kvPair_proto.txn_timestamp() << " Timestamp from block: " << kvPair.txn_timestamp << std::endl;
    }

    // google::protobuf::ShutdownProtobufLibrary();

    // Return to user
    return actualBlock;
}
