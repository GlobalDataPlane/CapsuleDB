#include <openssl/sha.h>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <sstream>
#include <iostream>
#include "capsuleBlock.hh"
#include "fakeCapsule.hh"
#include "capsuleBlock.pb.h""

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
std::string putCapsuleBlock(CapsuleBlock inputBlock) {
    // Serialize Block
    serialized::CapsuleBlock protobufBlock;
    protobufBlock.set_level(inputBlock.getLevel());
    protobufBlock.set_startkey(inputBlock.getMinKey());
    protobufBlock.set_endkey(inputBlock.getMaxKey());
    protobufBlock.mutable_kvpairs() = {inputBlock.getKVPairs().begin(),
                                         inputBlock.getKVPairs().end()};
    std::string serializedBlock;
    if (!protobufBlock.SerializeToString(serializedBlock.data())) {
        std::cerr << "Failed to serialize CapsuleBlock" << endl;
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
    sotredBlockFile.close();

    // return Hash
    return blockHash;
}

CapsuleBlock getCapsuleBlock(std::string inputHash) {
    serialized::CapsuleBlock recoveredBlock;
    // Retrieve and deserialize block
    std::ifstream storedBlock(inputHash);
    if (!recoveredBlock.ParseFromIstream(&storedBlock)) {
        std::cerr << "Failed to parse CapsuleBlock" << endl;
        return NULL;
    }

    // * Re-serialize and check hash *
    // ** Serialize Block **
    std::string serializedBlock;
    if (!protobufBlock.SerializeToString(serializedBlock.data())) {
        std::cerr << "Failed to serialize CapsuleBlock" << endl;
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
    CapsuleBlock actualBlock(serializedBlock.level());
    actualBlock.setMinKey(serializedBlock.startkey());
    actualBlock.setMaxKey(serializedBlock.endkey());
    for (const serialized::CapsuleBlock &kvPair_pb: serializedBlock.kvPairs) {

      struct kvs_payload kvPair = {kvPair_pb.key(), kvPair_pb.value(),
                                   kvPair_pb.txn_timestamp(),
                                   kvPair_pb.txn_msgtype()};
      actualBlock.addKVPair(kvPair);
    }

    // Return to user
    return actualBlock;
}
