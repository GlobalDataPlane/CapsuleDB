#ifndef CAPSULEBLOCK_H
#define CAPSULEBLOCK_H

#include <string>
#include <vector>
#include "src/shared/capsule.h"

typedef struct {
    std::string hash;
    std::string minKey;
    std::string maxKey;
} blockHeader;


class CapsuleBlock {
    public:
        int level;
        std::string startKey; // Defines the range of keys contained in this block
        std::string endKey;
        std::vector<kvs_payload> kvPairs; // Key, value, timestamp, msgType
        
        CapsuleBlock();
        CapsuleBlock(int l);
        int getLevel();
        std::string getMinKey();
        std::string getMaxKey();
        std::vector<kvs_payload> getKVPairs();
        void addKVPair(kvs_payload);
        void setMinKey(std::string k);
        void setMaxKey(std::string k);
        std::string writeOut();
        void readIn(std::string transactionHash, CapsuleBlock *location);
};

void readIn(std::string transactionHash, CapsuleBlock *location);

#endif