/* This file contains the disk / eventual DataCapsule interface for CapsuleDB */

#include "absl/strings/str_split.h"
#include "asylo/util/logging.h"
#include <cmath>
#include <exception>
#include <fstream>
#include <iostream>
#include <iterator>
#include <openssl/sha.h>
#include <sstream>
#include <stdexcept>
#include <vector>
#include "capsuleBlock.hh"
#include "fakeCapsule.hh"
#include "src/core/capsuleBlock.pb.h"
#include "src/core/kvs_payload.pb.h"


int counter = 0;

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
    outputBuffer[64] = '\0';
}

/*
    This function takes in a CapsuleBlock and writes it to disk.  
    
    Input: CapsuleBlock
    Output: std::string which is a SHA256 hash of the block and is the name of the file on disc
*/
std::string putCapsuleBlock(CapsuleBlock inputBlock)
{
    // Serialize Block
    capsuleDBProtos::CapsuleBlock protobufBlock;
    // TODO: Change this to something more robust than a counter
    protobufBlock.set_counter(counter);
    counter++;
    protobufBlock.set_level(inputBlock.getLevel());
    protobufBlock.set_startkey(inputBlock.getMinKey());
    protobufBlock.set_endkey(inputBlock.getMaxKey());

    for (int i = 0; i < inputBlock.kvPairs.size(); i++) {
        capsuleDBProtos::Kvs_payload* kvs_payload_serialized = protobufBlock.add_kvpairs();
        kvs_payload_serialized->set_key(inputBlock.kvPairs[i].key);
        kvs_payload_serialized->set_value(inputBlock.kvPairs[i].value);
        kvs_payload_serialized->set_txn_timestamp(inputBlock.kvPairs[i].txn_timestamp);
        kvs_payload_serialized->set_txn_msgtype(inputBlock.kvPairs[i].txn_msgType);
    }

    // Serialize
    std::string serializedBlock;
    bool success = protobufBlock.SerializeToString(&serializedBlock);
    if (!success) {
        std::cerr << "Failed to put CapsuleBlock with min key " << inputBlock.getMinKey() << " and max key " << inputBlock.getMaxKey() << std::endl;
        throw std::logic_error("String serialization failure.\n");
    }

    // Hash bytestream
    char blockHash[65];
    sha256_string(serializedBlock.data(), blockHash);

    // Store serialized block in file
    std::ofstream storedBlockFile;
    storedBlockFile.open(blockHash);
    success = protobufBlock.SerializeToOstream(&storedBlockFile);
    if (!success) {
        throw std::logic_error("Ostream serialization failure.\n");
    }
    storedBlockFile.close();

    // return Hash
    return blockHash;
}

/* 
    This function retrieves a CapsuleBlock stored on disk.

    Input: String that is the hash (and file name) of the requested CapsuleBlock
    Output: The requested CapsuleBlock
*/
CapsuleBlock getCapsuleBlock(std::string inputHash)
{
    capsuleDBProtos::CapsuleBlock recoveredBlock;

    // Retrieve and deserialize block
    std::ifstream storedBlock;
    storedBlock.open(inputHash);
    bool success = recoveredBlock.ParseFromIstream(&storedBlock);
    if (!success) {
        throw std::logic_error("Failed to deserialize retreived block.\n");
    }
    storedBlock.close();


    // Re-serialize and check hash
    std::string serializedBlock;
    success = recoveredBlock.SerializeToString(&serializedBlock);
    if (!success) {
        std::cerr << "Failed to string serialize retrieved CapsuleBlock." << std::endl;
        throw std::logic_error("SerializeToString failure in getCapsuleBlock.\n");
    }

    // Hash bytestream
    char blockHash[65];
    sha256_string(serializedBlock.data(), blockHash);

    // Verify hash
    if (blockHash != inputHash) {
        throw std::invalid_argument("Hash mismatch on retrieval, possible tampering detected.\n");
    }

    // Convert recoveredBlock(proto) to actual CapsuleBlock
    CapsuleBlock actualBlock(recoveredBlock.level());
    actualBlock.setMinKey(recoveredBlock.startkey());
    actualBlock.setMaxKey(recoveredBlock.endkey());

    for (int i = 0; i < recoveredBlock.kvpairs_size(); i++) {
        kvs_payload retrieved_payload;
        retrieved_payload.key = recoveredBlock.kvpairs(i).key();
        retrieved_payload.value = recoveredBlock.kvpairs(i).value();
        retrieved_payload.txn_timestamp = recoveredBlock.kvpairs(i).txn_timestamp();
        retrieved_payload.txn_msgType = recoveredBlock.kvpairs(i).txn_msgtype();
        actualBlock.addKVPair(retrieved_payload);
    }

    // Return to user
    return actualBlock;
}
