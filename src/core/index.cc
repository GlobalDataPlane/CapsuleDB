/*
 * This file manages and generates new indices for capsuleDB.  It can produce both full and partial indices.
 */

#include <string>
#include <tuple>
#include <cmath>
#include <vector>
#include "../bloom/bloom_filter.hpp"
#include "capsuleBlock.hh"
#include "index.hh"
#include "level.hh"
#include <iostream>

CapsuleIndex::CapsuleIndex() {
    CapsuleIndex(-1);
}

CapsuleIndex::CapsuleIndex(size_t size) {
    numLevels = 1;
    blocksize = size;
    // TODO: prevIndexHash???
    Level level_zero = Level(0, 2 * blocksize);
    levels.push_back(level_zero);
}

/*
* Returns the number of levels in the database.
*
* Output: int which is the number of levels.
*/
int CapsuleIndex::getNumLevels() {
    return numLevels;
}

/*
    * Returns the hash of the block with the requested key on a given level.  
    *
    * Input: Level and key
    * Output: block hash or error code
    */
std::string CapsuleIndex::getBlock(int level, std::string key) {
    #ifdef DEBUG
    std::cout << "getBlock on level=" << level << " for key=" << key << "\n";
    #endif

    if (level < 0 || level >= numLevels) {
        return "";
    }
    Level curr_level = levels[level];
    // if (curr_level.levelFilter.contains(key)) {
    //     return curr_level.getBlock(key);
    // }
    // return "";
    return curr_level.getBlock(key);
}

int CapsuleIndex::add_hash(int level, std::string hash, CapsuleBlock block) {
    if (level < 0 || level >= numLevels) {
        return -1;
    }
    int status = levels[level].addBlock(&block, hash);
    compact();
    return status;
}

/* 
    * Creates a new level and appends it to the index.
    * 
    * Input: Size of the level (Stored in vector in capsuledb.cc)
    * Output: Returns the index of the new level.
    */
int CapsuleIndex::addLevel(int size) {
    Level newLevel = Level(numLevels, size);
    newLevel.numBlocks = 0;
    newLevel.maxSize = size;
    newLevel.index = numLevels;

    levels.push_back(newLevel);
    
    // newLevel.levelFilter = newLevel.create_filter();

    numLevels++;
    return numLevels - 1; 
}

/*
* This function manages the compaction process by performing initial comapction determinations at L0
* Further compactions and the actual compaction logic is handled in compactHelper.  
* 
* Input: Level to compact.
* Output: Error code or 0 on success.
*/
int CapsuleIndex::compact() {

    #ifdef DEBUG
    std::cout << "ENTERING compact()\n";

    std::cout << "blocksize=" << blocksize << "\n";
    std::cout << "lv0->numBlocks=" << levels[0].numBlocks << "\n";
    std::cout << "lv0->maxSize=" << levels[0].maxSize << "\n";
    #endif

    if (blocksize * levels[0].numBlocks < levels[0].maxSize) {
        return 0;
    }

    #ifdef DEBUG
    std::cout << "recordHashes.size()=" << levels[0].recordHashes.size() << "\n";
    #endif

    // Sort vectors in each block of L0
    sortL0();

    // Sort L0
    std::vector<blockHeader> sortedLv0;
    sortedLv0.push_back(levels[0].recordHashes[0]);
    for (int i = 1; i < levels[0].recordHashes.size(); i++) {
        std::vector<blockHeader> currBlock;
        currBlock.push_back(levels[0].recordHashes[i]);
        sortedLv0 = merge(sortedLv0, currBlock, 0);
    }
    
    #ifdef DEBUG
    std::cout << "sortedLv0.size()=" << sortedLv0.size() << "\n";
    #endif


    if (numLevels == 1) {
        addLevel(10 * levels[0].maxSize);
    }

    compactHelper(sortedLv0, 1);    
    

    #ifdef DEBUG
    std::cout << "Size of L1 after return from compactHelper=" << levels[1].recordHashes.size() << "\n";
    std::cout << "min_key of L1 after return from compactHelper=" << levels[1].min_key << "\n";
    #endif
    
    levels[0].recordHashes.clear();
    levels[0].numBlocks = 0;
    levels[0].min_key = "";
    levels[0].max_key = "";

    return 0;
}

/* 
 * This function merges blocks into exisiting levels.  It also determines whether doing so would cause an overflow.
 * If so, it recursively compacts by identifying which blocks are modified in destLevel and pushing them into the level
 * below.
 * 
 * Input: A sorted vector of blockHeaders sourceVec, the level the vector is being compacted into destLevel.
 * Output: 0 if no error, other number otherwise
 */

int CapsuleIndex::compactHelper(std::vector<blockHeader> sourceVec, int destLevelInd) {
    
    #ifdef DEBUG
    std::cout << "ENTERING compactHelper()\n";
    std::cout << "sourceVec.size()=" << sourceVec.size() << "\n";
    std::cout << "destLevel.recordHashes.size()=" << destLevel->recordHashes.size() << "\n";
    std::cout << "destLevel.maxSize=" << destLevel->maxSize << "\n";
    #endif
    
    if (blocksize * sourceVec.size() + blocksize * levels[destLevelInd].recordHashes.size() >= levels[destLevelInd].maxSize) {
        // Add new level
        if (levels[destLevelInd].index + 1 >= numLevels) {
            addLevel(levels[destLevelInd].maxSize * 10);
        }

        // Identify Affected blocks
        std::vector<blockHeader> newSourceVec;
        std::vector<blockHeader> remainingBlocks;
        int sourceInd = 0;
        int destInd = 0;

        while (sourceInd < sourceVec.size() && destInd < levels[destLevelInd].recordHashes.size()) {
            if (sourceVec[sourceInd].minKey > levels[destLevelInd].recordHashes[destInd].minKey 
                && sourceVec[sourceInd].minKey > levels[destLevelInd].recordHashes[destInd].maxKey) {
                while (destInd < levels[destLevelInd].recordHashes.size()
                       && sourceVec[sourceInd].maxKey > levels[destLevelInd].recordHashes[destInd].maxKey) {
                    newSourceVec.push_back(levels[destLevelInd].recordHashes[destInd]);
                    destInd++;
                }
                sourceInd++;
            } else {
                remainingBlocks.push_back(levels[destLevelInd].recordHashes[destInd]);
                destInd++;
            }
        }
        
        while (destInd < levels[destLevelInd].recordHashes.size()) {
            remainingBlocks.push_back(levels[destLevelInd].recordHashes[destInd]);
            destInd++;
        }

    
        // Select additional blocks if not enough are evicted
        if (newSourceVec.size() < sourceVec.size()) {
            auto it = std::next(remainingBlocks.begin(), sourceVec.size() - newSourceVec.size());
            std::move(remainingBlocks.begin(), it, std::back_inserter(newSourceVec));
            remainingBlocks.erase(remainingBlocks.begin(), it);
            std::sort(newSourceVec.begin(), newSourceVec.end(), compareHeaders);
        }

        compactHelper(newSourceVec, destLevelInd + 1);

        if (remainingBlocks.size() == 0) {
            levels[destLevelInd].recordHashes.clear();
            levels[destLevelInd].numBlocks = 0;
            levels[destLevelInd].min_key = "";
            levels[destLevelInd].max_key = "";
        } else {
            levels[destLevelInd].recordHashes = remainingBlocks;
            levels[destLevelInd].numBlocks = remainingBlocks.size();
            levels[destLevelInd].min_key = remainingBlocks[0].minKey;
            levels[destLevelInd].max_key = remainingBlocks[remainingBlocks.size() - 1].maxKey;
        }
    }
    
    std::vector<blockHeader> newDestLevelVec = merge(sourceVec, levels[destLevelInd].recordHashes, destLevelInd);
    if (newDestLevelVec.size() == 0) {
        levels[destLevelInd].recordHashes.clear();
        levels[destLevelInd].numBlocks = 0;
        levels[destLevelInd].min_key = "";
        levels[destLevelInd].max_key = "";
    } else {
        levels[destLevelInd].recordHashes = newDestLevelVec;
        levels[destLevelInd].numBlocks = newDestLevelVec.size();
        levels[destLevelInd].min_key = newDestLevelVec[0].minKey;
        levels[destLevelInd].max_key = newDestLevelVec[newDestLevelVec.size() - 1].maxKey;
    }

    return 0;
}

/*
    This function takes in two lists of blockHeaders representing two levels of sorted CapsuleBlocks
    Merges them into a new list of blockHeaders representing a level of sorted CapsuleBlocks.
*/
std::vector<blockHeader> CapsuleIndex::merge(std::vector<blockHeader> a, std::vector<blockHeader> b, int next_level) {
    #ifdef DEBUG
    std::cout << "ENTERING merge()\n";
    #endif
    
    std::vector<blockHeader> output;
    
    size_t aa = 0;
    size_t bb = 0;
    size_t aaa = 0;
    size_t bbb = 0;

    bool advanceA = false;
    bool advanceB = false;

    blockHeader blockHeaderA = a[aa];
    CapsuleBlock capsuleBlockA;
    readIn(blockHeaderA.hash, &capsuleBlockA);

    blockHeader blockHeaderB;
    CapsuleBlock capsuleBlockB;
    if (b.size() > 0) {
        blockHeaderB = b[bb];
        readIn(blockHeaderB.hash, &capsuleBlockB);
    }
    
    CapsuleBlock next_cb = CapsuleBlock(next_level);

    while (aa != a.size() || bb != b.size()) {

        // Find smallest KV pair by comparing the next KV pair of each level.
        kvs_payload nextKVPair;

        // If we've reached the end of either level, use other level.
        if (aa == a.size()) {
            nextKVPair = capsuleBlockB.getKVPairs()[bbb];
            advanceB = true;
        }
        else if (bb == b.size()) {
            nextKVPair = capsuleBlockA.getKVPairs()[aaa];
            advanceA = true;
        } else {
            // If both levels have remaining KV pairs to add, push out the smallest one.
            kvs_payload KVPairA = capsuleBlockA.getKVPairs()[aaa];
            kvs_payload KVPairB = capsuleBlockB.getKVPairs()[bbb];
            std::string keyA = KVPairA.key;
            std::string keyB = KVPairB.key;
            if (keyB < keyA) {
                nextKVPair = KVPairB;
                advanceB = true;
            } else if (keyA < keyB) {
                nextKVPair = KVPairA;
                advanceA = true;
            } else {
                // If both KV pairs have the same key, push out the most recent one and disregard the older one.
                int64_t timestampA = KVPairA.txn_timestamp;
                int64_t timestampB = KVPairB.txn_timestamp;
                if (timestampB >= timestampA) {
                    nextKVPair = KVPairB;
                } else {
                    nextKVPair = KVPairA;
                }
                advanceA = true;
                advanceB = true;
            }
        }
        
        // Advance pointers for either A or B or both, pulling in next CapsuleBlock if reached end of current one.
        if (advanceA) {
            aaa++;
            if (aaa == capsuleBlockA.getKVPairs().size()) {
                aaa = 0;
                aa++;
                if (aa < a.size()) {
                    blockHeaderA = a[aa];
                    readIn(blockHeaderA.hash, &capsuleBlockA);
                }
            }
        }
        if (advanceB) {
            bbb++;
            if (bbb == capsuleBlockB.getKVPairs().size()) {
                bbb = 0;
                bb++;
                if (bb < b.size()) {
                    blockHeaderB = b[bb];
                    readIn(blockHeaderB.hash, &capsuleBlockB);
                }
            }
        }
        advanceA = false;
        advanceB = false;

        // Add next KV pair to a CapsuleBlock, and write out if full.
        #ifdef DEBUG
        std::cout << "next key=" << std::get<0>(nextKVPair) << "\n";
        #endif
        
        next_cb.addKVPair(nextKVPair);
        if (next_cb.getKVPairs().size() == blocksize) {
            std::string hash = next_cb.writeOut();
            blockHeader bh = {};
            bh.hash = hash;
            bh.minKey = next_cb.getMinKey();
            bh.maxKey = next_cb.getMaxKey();
            output.push_back(bh);
            next_cb = CapsuleBlock(next_level);
        }
    }

    // Write out the final CapsuleBlock, even if it's not full.
    if (next_cb.getKVPairs().size() > 0) {
        std::string hash = next_cb.writeOut();
        blockHeader bh = {};
        bh.hash = hash;
        bh.minKey = next_cb.getMinKey();
        bh.maxKey = next_cb.getMaxKey();
        output.push_back(bh);
    }

    return output;
}

/*
 * This function sorts the keys in each individual L0 block.
 *
 * Input: None
 * Output: None
 */
void CapsuleIndex::sortL0() {
    #ifdef DEBUG
    std::cout << "ENTERING sortL0()" << "\n";
    #endif
    Level* lv0 = &levels.front();
    CapsuleBlock currBlock;
    std::vector<blockHeader> newRecordHashes;
    for (int i = 0; i < lv0->recordHashes.size(); i++) {
        readIn(lv0->recordHashes[i].hash, &currBlock); 
        std::sort(currBlock.kvPairs.begin(), currBlock.kvPairs.end(), comparePayloads);
        std::string hash = currBlock.writeOut();
        lv0->recordHashes[i].hash = hash;
    }
}

/*
 * Compares two kvs_payloads to determine which is less than the other.
 * 
 * Input: Two payloads to compare
 * Output: True if payloadOne < payloadTwo, false otherwise
 */
bool comparePayloads (kvs_payload payloadOne, kvs_payload payloadTwo) {
    return payloadOne.key < payloadTwo.key;
}

/*
 * Compares two blockHeaders to determine which comes first when ordered.
 * WARNING: Only use for non-overlapping blockHeaders.
 * 
 * Input: Two headers to compare
 * Output: True if headerOne comes before headerTwo, false otherwise
 */
bool compareHeaders (blockHeader headerOne, blockHeader headerTwo) {
    return headerOne.minKey < headerTwo.minKey;
}