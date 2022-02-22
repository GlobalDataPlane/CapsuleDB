#include <string>
#include <tuple>
#include <vector>
#include <iostream>
#include "../bloom/bloom_filter.hpp"
#include "capsuleBlock.hh"
#include "fakeCapsule.hh"
#include "level.hh"

Level::Level() {
    Level(-1, -1);
}

Level::Level(int i, int ms) {
    index = i;
    maxSize = ms;
    // levelFilter = create_filter();

    numBlocks = 0;
    min_key = "";
    max_key = "";
}

bloom_filter Level::create_filter() {
    bloom_parameters params;
    params.projected_element_count = 750000;
    params.false_positive_probability = 0.05;
    params.compute_optimal_parameters();
    bloom_filter filter(params);
    return filter;
}

/*
* Returns the number of blocks in this level.
* 
* Output: int representing number of blocks in the level
*/
int Level::getNumBlocks() {
    return numBlocks;
}

/*
* Sets the number of blocks in this level.
*/
void Level::setNumBlocks(int n) {
    numBlocks = n;
}

/*
* Adds a new block to the level.  Generates a quotient filter and logs the
* range of keys in the block as well.
* 
* Input: New CapsuleBlock to be added.
* Output: 0 on success, other int on error.
*/
int Level::addBlock(CapsuleBlock* newBlock, std::string hash) {
    #ifdef DEBUG
    std::cout << "addBlock on level=" << index << "\n";
    #endif
    // Add kv pairs in block to bloom filter
    std::vector <kvs_payload> kvPairs = newBlock->getKVPairs();
    for (kvs_payload kvt : kvPairs) {
        std::string key = kvt.key;
        // std::cout << "levelFilter.insert " << key << "\n";
        // levelFilter.insert(key);
    }

    std::string new_block_min_key = (*newBlock).getMinKey();
    std::string new_block_max_key = (*newBlock).getMaxKey();

    // If level is empty, directly add and return.
    if (min_key == "" || max_key == "") {
        numBlocks++;
        min_key = new_block_min_key;
        max_key = new_block_max_key;
        blockHeader bh = {hash, new_block_min_key, new_block_max_key};
        recordHashes.push_back(bh);
        return 0;
    }
    
    // If this is L0, just append (L0 is unsorted)
    if (index == 0) {
        blockHeader bh = {hash, new_block_min_key, new_block_max_key};
        recordHashes.push_back(bh);
        numBlocks++;
        min_key = min(std::string(min_key), std::string(new_block_min_key));
        max_key = max(std::string(max_key), std::string(new_block_max_key));
        return 0;
    }

    return -1;
}

/*
* Pulls the corresponding CapsuleBlock hash for the provided key.  First checks keyRanges to find 
* the index of the corresponding block.  Then checks the block's quotient filter
* to estimate membership.  Finally, pulls the relevant block hash.
* 
* Input: Desired key
* Output: The hash which potentially contains the requested key, error code if not present
*/
std::string Level::getBlock(std::string key) {
    #ifdef DEBUG
    std::cout << "getBlock on level=" << index << " for key=" << key << "\n";
    std::cout << "Level min_key=" << min_key << "\n";
    std::cout << "Level max_key=" << max_key << "\n";
    std::cout << "Level size=" << recordHashes.size() << "\n";
    #endif

    if (key < min_key || key > max_key) {
        return "";
    }
    // Otherwise search -> is Binary really needed?
    
    for (int i = 0; i < numBlocks; i++) {
        #ifdef DEBUG
        std::cout << "recordHashes[" << i << "].hash=" << recordHashes[i].hash << "\n";
        #endif

        if (recordHashes[i].minKey <= key && key <= recordHashes[i].maxKey) {
            return recordHashes[i].hash;
        }
    }
    return "";
}
