#ifndef LEVEL_H
#define LEVEL_H

#include <string>
#include <vector>
#include "src/bloom/bloom_filter.hpp"
#include "capsuleBlock.hh"

class Level {  
    public:
        int index;
        int numBlocks;
        int maxSize;
        std::string min_key;
        std::string max_key;
        std::vector <blockHeader> recordHashes;
        // bloom_filter levelFilter;


        Level();
        Level(int i, int ms);
        bloom_filter create_filter();
        int getNumBlocks();
        void setNumBlocks(int n);
        int addBlock(CapsuleBlock* newBlock, std::string hash);
        std::string getBlock(std::string key);
};

#endif