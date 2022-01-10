#include "memtable_new.hpp"

#include <iostream>
#include "asylo/util/logging.h"
#include "capsuleBlock.hh"
#include "level.hh"

Memtable::Memtable() {
    Memtable(-1);
}

Memtable::Memtable(size_t ms) {
    max_size = ms;
}

kvs_payload Memtable::get(const std::string &key)
{
    // First check if a lock is present. If not, key is not present and can return.
    // If present, wait to get lock and access the data item.
    kvs_payload got;
    if (!locklst.contains(key))
    {
        #ifdef DEBUG
        std::cout << "Memtable: Couldn't find key: " << key << "\n";
        #endif
        got.key = "";
    }
    else
    {
        std::mutex *lock = locklst.at(key);
        lock->lock();
        got = memtable.at(key);
        lock->unlock();
    }
    return got;
}
/* This function finds whether a lock is already present and if not creates a lock and adds to lock list.
 * The lock is then acquired and modifications done to the value.
 * Main philosophy is that concurrent reads and writes on different key values should not stall on the single memtable lock.
 */
bool Memtable::put(const kvs_payload *payload, CapsuleIndex* index)
{
    auto prev_iter_lock = locklst.find(payload->key);
    if (prev_iter_lock != locklst.end())
    {
        // key already exists
        std::mutex *lock = prev_iter_lock->second;
        lock->lock();
        auto prev_iter = memtable.find(payload->key);
        // No need to check prev_iter with end since locklst and memtable keys are synchronized
        int64_t prev_timestamp = prev_iter->second.txn_timestamp;
        //the timestamp of this payload is earlier, skip the change
        if (payload->txn_timestamp <= prev_timestamp)
        {
            #ifdef DEBUG
            std::cout << "[EARLIER DISCARDED] Timestamp of incoming payload key: " << payload->key
                 << ", timestamp: " << payload->txn_timestamp << " ealier than " << prev_timestamp;
            #endif
           lock->unlock();
           return false;
        }
        else
        {
            memtable[payload->key] = *payload;
            #ifdef DEBUG
            std::cout << "[SAME PAYLOAD UPDATED] Timestamp of incoming payload {key=" << payload->key
                 << ", timestamp=" << payload->txn_timestamp << "} replaces previous timestamp=" << prev_timestamp << "\n";
            #endif
            lock->unlock();
            return true;
        }
    }
    else
    {
        write_out_if_full(index);
        // key does not exist, create spinlock object and add to lock list.
        std::mutex *lock = new std::mutex();
        locklst[payload->key] = lock; // add new lock to locklst
        lock->lock();
        memtable[payload->key] = *payload;
        lock->unlock();
        return true;
    }
}

/* This function writes out entire memtable to level 0 of tree if the number of kv pairs is at capacity.
 */
void Memtable::write_out_if_full(CapsuleIndex* index)
{
    // capacity check: number of kv pairs (upperbounds amount of memory when we constrain kv size)
    // std::cout << "write_out_if_full: " << "max_size=" << max_size << " memtable.size()=" << memtable.size() << "\n";

    if (memtable.size() >= max_size)
    {
        // Level* level_zero = &index->levels.front();
        // std::cout << "level_zero->index=" << level_zero->index << "\n";
        // std::cout << "level_zero->numBlocks=" << level_zero->numBlocks << "\n";
        // std::cout << "level_zero->maxSize=" << level_zero->maxSize << "\n";
        // std::cout << "level_zero->min_key=" << level_zero->min_key << "\n";


        CapsuleBlock capsule_block(0);

        // initialize min/max
        std::string min_key = memtable.begin()->first;
        std::string max_key = memtable.begin()->first;

        for (const auto &p : memtable)
        {
            kvs_payload payload = p.second;
            capsule_block.addKVPair(payload);
            min_key = min(std::string(min_key), std::string(p.first));
            max_key = max(std::string(max_key), std::string(p.first));
        }

        capsule_block.setMinKey(min_key);
        capsule_block.setMaxKey(max_key);

#ifdef DEBUG
        std::cout << "Memtable filled, writing out block with min_key=" << min_key << ", max_key=" << max_key << "\n";
#endif
        std::string record_hash = capsule_block.writeOut();
        // std::string record_hash = "temp hash";

        index->add_hash(0, record_hash, capsule_block);
        // level_zero->addBlock(&capsule_block, record_hash);

        memtable = {};
        locklst = {};
    }
}
