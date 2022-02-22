#ifndef INDEX_H
#define INDEX_H

#include <string>
#include <vector>
#include "capsuleBlock.hh"
#include "level.hh"

class CapsuleIndex {
    public:
        int numLevels;
        size_t blocksize;
        std::string prevIndexHash;
        std::vector <Level> levels;
        
        CapsuleIndex();
        CapsuleIndex(size_t size);
        int getNumLevels();
        std::string getBlock(int level, std::string key);
        int add_hash(int level, std::string hash, CapsuleBlock block);
        int addLevel(int size);
        int compact();
        int compactHelper(std::vector<blockHeader> sourceVec, int destLevelInd);
        std::vector<blockHeader> merge(std::vector<blockHeader> a, std::vector<blockHeader> b, int next_level);
        void sortL0();
};
bool comparePayloads (kvs_payload payloadOne, kvs_payload payloadTwo);
bool compareHeaders (blockHeader headerOne, blockHeader headerTwo);

#endif