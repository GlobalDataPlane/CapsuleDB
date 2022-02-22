#ifndef _MEMTBL_H
#define _MEMTBL_H

#include "absl/container/flat_hash_map.h"
#include "../common.h"
#include "../kvs_include/capsule.h"
#include <mutex>
#include "index.hh"
#include <map>
//#include "../sgx_spinlock.h"

class Memtable
{
public:
    size_t max_size;
    bool put(const kvs_payload *payload, CapsuleIndex* index);
    kvs_payload get(const std::string &key);
    void write_out_if_full(CapsuleIndex* index);
    Memtable();
    Memtable(size_t ms);
    // M_BENCHMARK_CODE
private:
    std::map<std::string, kvs_payload> memtable;
    std::map<std::string, std::mutex *> locklst; // each kv has its own lock.
    // absl::btree_set<std::string> sort_cache;                    // stores sorted set of keys to be used when moved to upper levels.(optimization)
};

#endif
