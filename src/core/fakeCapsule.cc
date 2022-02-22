#include <string>
#include <fstream>
#include <openssl/sha.h>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <sstream>
#include "capsuleBlock.hh"
#include "fakeCapsule.hh"
#include <iostream>

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

CapsuleBlock getCapsuleBlock(std::string inputHash) {
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
