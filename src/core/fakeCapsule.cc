#include <openssl/sha.h>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <sstream>
#include <iostream>
#include <fstream>
#include "capsuleBlock.hh"
#include "fakeCapsule.hh"
#include "src/core/capsuleBlock.pb.h"

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
    for (auto it = inputBlock.getKVPairs().begin(); it != inputBlock.getKVPairs().end(); it++) {
        capsuleDBSerialization::CapsuleBlock::kvs_payload* kvPair_proto = protobufBlock.add_kvpairs();
        kvPair_proto->set_key(it->key);
        kvPair_proto->set_value(it->value);
        kvPair_proto->set_txn_timestamp(it->txn_timestamp);
        kvPair_proto->set_txn_msgtype(it->txn_msgType);
    }

    std::string serializedBlock;
    if (!protobufBlock.SerializeToString(&serializedBlock)) {
        std::cerr << "Failed to serialize CapsuleBlock" << std::endl;
        return "";
    }
    // std::cout << "putCapsuleBlock: toBeHashed=" << serializedBlock << "\n";

    // Hash bytestream
    char blockHash[65];
    sha256_string(serializedBlock.data(), blockHash);
    // std::cout << "putCapsuleBlock: blockHash=" << blockHash << "\n";

    // Store serialized block in file
    std::ofstream storedBlockFile(blockHash);
    storedBlockFile << serializedBlock;
    storedBlockFile.close();

    google::protobuf::ShutdownProtobufLibrary();

    // return Hash
    return blockHash;
}

CapsuleBlock getCapsuleBlock(std::string inputHash)
{
    capsuleDBSerialization::CapsuleBlock recoveredBlock;
    // Retrieve and deserialize block
    std::ifstream storedBlock(inputHash);
    if (!recoveredBlock.ParseFromIstream(&storedBlock)) {
        std::cerr << "Failed to parse CapsuleBlock" << std::endl;
        return NULL;
    }

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
      const capsuleDBSerialization::CapsuleBlock::kvs_payload& kvPair_proto =
          recoveredBlock.kvpairs(i);
      kvs_payload kvPair = {kvPair_proto.key(), kvPair_proto.value(),
                            kvPair_proto.txn_timestamp(),
                            kvPair_proto.txn_msgtype()};
      actualBlock.addKVPair(kvPair);
    }

    google::protobuf::ShutdownProtobufLibrary();

    // Return to user
    return actualBlock;
}
