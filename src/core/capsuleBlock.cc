/* 
 * This file defines a block of key-value pairs which is stored in a DataCapsule.  
 */

#include <string>
#include <vector>
#include <iostream>
#include "capsuleBlock.hh"
#include "../kvs_include/capsule.h"
#include "fakeCapsule.hh"

CapsuleBlock::CapsuleBlock() {
    CapsuleBlock(-1);
}

CapsuleBlock::CapsuleBlock(int l) {
    level = l;
    startKey = "";
    endKey = "";
}

/*
    * Returns the level of the capsule block
    *
    * Output: int representing the block's level
    */
int CapsuleBlock::getLevel()
{
    return level;
}

/*
    * Return the lower bound of keys in this block
    *
    * Output: int representing the lowest key in the block
    */
std::string CapsuleBlock::getMinKey()
{
    return startKey;
}

/*
    * Return the upper bound of keys in this block
    *
    * Output: int representing the highest key in the block
    */
std::string CapsuleBlock::getMaxKey()
{   
    return endKey;
}

std::vector<kvs_payload> CapsuleBlock::getKVPairs()
{
    return kvPairs;
}

void CapsuleBlock::addKVPair(kvs_payload element)
{
    kvPairs.push_back(element);
    if (startKey == "") {
        startKey = element.key;
    }
    if (endKey == "") {
        endKey = element.key;
    }
    startKey = min(std::string(startKey), std::string(element.key));
    endKey = max(std::string(endKey), std::string(element.key));
}

/*
    * Set the lower bound of keys in this block
    */
void CapsuleBlock::setMinKey(std::string k)
{
    startKey = k;
}

/*
    * Set the upper bound of keys in this block
    */
void CapsuleBlock::setMaxKey(std::string k)
{
    endKey = k;
}


/*
    * This function takes a prepared block and pushes it to the DataCapusle.
    *
    * Input: None
    * Output: DataCapsule record hash.
    */
std::string CapsuleBlock::writeOut()
{
    return putCapsuleBlock(*this);
}

/*
    * This function reads in a data block from the DataCapusle.
    * 
    * Input: Transaction hash of the requested block and the memory location for the record to be stored.
    * Output: Error code or 0 on success.
    */
void readIn(std::string transactionHash, CapsuleBlock *location)
{
    *location = getCapsuleBlock(transactionHash);
}
