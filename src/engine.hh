#ifndef ENGINE_H
#define ENGINE_H

/*
 * This file manages the database as well as read/write requests.  
 */

#include <string>
#include <vector>
#include <map>
#include "memtable_new.hpp"
#include "absl/strings/string_view.h"
#include "index.hh"
#include "kvs_include/capsule.h"
//#include "../benchmark.h"

// using namespace asylo;

class CapsuleDB {
    public:
        std::map <std::string, std::string> test_map;

        std::string name;
        int maxLevels = 5;
        std::string targetCapsule;
        int test_count=0;
        std::vector<int> maxLevelSizes; //Each level in bytes
        const absl::string_view signing_key_pem = {
            R"pem(-----BEGIN EC PRIVATE KEY-----
            MHcCAQEEIF0Z0yrz9NNVFQU1754rHRJs+Qt04mr3vEgNok8uyU8QoAoGCCqGSM49
            AwEHoUQDQgAE2M/ETD1FV9EFzZBB1+emBFJuB1eh2/XyY3ZdNrT8lq7FQ0Z6ENdm
            oG+ldQH94d6FPkRWOMwY+ppB+SQ8XnUFRA==
            -----END EC PRIVATE KEY-----)pem"
        };
        Memtable memtable;
        CapsuleIndex index;
        
        CapsuleDB();
        kvs_payload get(const std::string &key, bool isMulticast = false);
        void put(const kvs_payload *payload);
        
        
        
        //M_BENCHMARK_HERE
	#include "benchmark.h"
        void benchmark_put(std::string key, std::string value)
        {
            test_map.insert({key, value});
            kvs_payload kvs;
            kvs.key = key;
            kvs.value = value;

            kvs.txn_timestamp = std::chrono::system_clock::to_time_t(
                           std::chrono::system_clock::now());
            put(&kvs);
        }
        void benchmark_get(std::string key)
        {
            get(key);
        }

        void benchmark_verify() {
            int num_found = 0;
            std::vector<std::string> failed_keys; 
            for(const auto& [key, value] : test_map) {
                kvs_payload payload = get(key);
                std::string value1 = payload.value;
                if(value1==""){
                    failed_keys.push_back(key);
                    // std::cout << key << "not found in capsuleDB";
                    test_count++;
                }
                if(value1 == value){
                    num_found++;
                }
            }
            std::cout << "Num levels at end: " << index.levels.size() << "\n";
            std::cout << "First key l0: " << index.levels[0].recordHashes[0].minKey << "\n";
            std::cout << "Last key l0: " << index.levels[0].recordHashes[index.levels[0].recordHashes.size() - 1].maxKey << "\n";
            std::cout << "L0 max size: " << index.levels[0].maxSize << "\n";
            std::cout << "First key l1: " << index.levels[1].recordHashes[0].minKey << "\n";
            std::cout << "Last key l1: " << index.levels[1].recordHashes[index.levels[1].recordHashes.size() - 1].maxKey << "\n";
            std::cout << "L1 max size: " << index.levels[1].maxSize << "\n";
            std::cout << "First key l2: " << index.levels[2].recordHashes[0].minKey << "\n";
            std::cout << "Last key l2: " << index.levels[2].recordHashes[index.levels[1].recordHashes.size() - 1].maxKey << "\n";
            std::cout << "L2 max size: " << index.levels[2].maxSize << "\n";
            // std::cout << "Key not found: " << failed_keys[1] << "\n";
            std::cout << "no.of.keys not found is:"<<test_count << "\n";
            std::cout << "no.of.keys found is:"<<num_found <<"\n";
            std::cout << "size of test_map: " <<test_map.size() <<"\n"; 
        //    for(int x=0; x <failed_keys.size();x++)
        //         std::cout << failed_keys.at(x)<<" ";
        }

        void benchmark2(){
            for(int i=1; i<2000; i++)
            {
                test_map.insert({std::to_string(i),std::to_string(i)});
                kvs_payload kvs;
                kvs.key = std::to_string(i);
                kvs.value = std::to_string(i);

                kvs.txn_timestamp = std::chrono::system_clock::to_time_t(
                               std::chrono::system_clock::now());
                put(&kvs);
            }
        }
};

/*
 * This function creates a new CapsuleDB instance.  It takes in information about the sizes of levels and other metadata to establish compaction rules.
 * 
 * Inputs: ???
 * Outputs: An error code
 */
CapsuleDB spawnDB(size_t memtable_size);
int connectDB();

#endif
