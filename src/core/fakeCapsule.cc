#include "absl/strings/str_split.h"
#include "asylo/util/logging.h"
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <fstream>
#include <iostream>
#include <openssl/sha.h>
#include <sstream>
#include <vector>
#include "capsuleBlock.hh"
#include "fakeCapsule.hh"
#include "src/core/capsuleBlock.pb.h"
#include "src/uuid_v4/uuid_v4.h"

/* For serializing kvs_payloads */
std::string delim_str = "@@@";
char delim = ';';

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

void KvToPayload(kvs_payload *payload, const std::string &key, const std::string &value, const int64_t timer,
                const std::string &msgType) {
    payload->key = key;
    payload->value = value;
    payload->txn_timestamp = timer;
    payload->txn_msgType = msgType;
}

std::string serialize_payload_l(const std::vector<kvs_payload> &payload_l) {
    std::string payload_l_s;
    for (const kvs_payload &payload : payload_l) {
        payload_l_s += std::to_string(payload.txn_timestamp) + delim_str +
                    payload.txn_msgType + delim_str + payload.key + delim_str +
                    payload.value + delim_str;
    }
    return payload_l_s;
}

std::vector<kvs_payload> deserialize_payload_l(const std::string &payload_l_s) {
    std::vector<kvs_payload> payload_l;
    std::stringstream ss(payload_l_s);
    std::string txn_timestamp, txn_msgType, key, value;

    std::vector<std::string> split = absl::StrSplit(payload_l_s, delim_str);

    if ((split.size() - 1) % 4 != 0) {
        LOG(ERROR) << "invalid payload size " << split.size();
        for (int i = 0; i < split.size(); i += 1) {
            LOG(ERROR) << i << " " << split[i];
        }
        return payload_l;
    }

    for (int i = 0; i < ((split.size() - 1) / 4) * 4; i += 4) {
        kvs_payload payload;
        txn_timestamp = split.at(i);
        txn_msgType = split.at(i + 1);
        key = split.at(i + 2);
        value = split.at(i + 3);

        KvToPayload(&payload, key, value, std::stoi(txn_timestamp), txn_msgType);
        payload_l.push_back(payload);
    }

    return payload_l;
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
      
    std::cout << "Test 3" << std::endl;

    std::vector<kvs_payload> pairs = inputBlock.getKVPairs();
    std::string pairs_str = serialize_payload_l(pairs);
    protobufBlock.set_kvpairs(pairs_str);


    std::cout << "Test 4" << std::endl;
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

    std::vector<kvs_payload> kvPairs = deserialize_payload_l(recoveredBlock.kvpairs());
    for (auto it = kvPairs.begin(); it != kvPairs.end(); it++) {
        actualBlock.addKVPair(*it);
    }

    // google::protobuf::ShutdownProtobufLibrary();

    // Return to user
    return actualBlock;
}
