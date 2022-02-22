#ifndef LEVEL_H
#define LEVEL_H

#include <string>
#include <vector>
#include "../bloom/bloom_filter.hpp"
#include "capsuleBlock.hh"

class Level {  
    private:
        friend class boost::serialization::access;
        // When the class Archive corresponds to an output archive, the
        // & operator is defined similar to <<.  Likewise, when the class Archive
        // is a type of input archive the & operator is defined similar to >>.
        template<class Archive>
        void serialize(Archive & ar, const unsigned int version)
        {
            ar & index;
            ar & numBlocks;
            ar & maxSize;
            ar & min_key;
            ar & max_key;
            ar & recordHashes;
        }

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