#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "src/core/engine.hh"
#include "src/shared/capsule.h"


int main()
{
    CapsuleDB* instance = spawnDB(50);
    int test_count = 0;
    std::map <std::string, std::string> test_map;

    // Put keys into database
    for(int i=1; i<2000; i++) {
        test_map.insert({std::to_string(i),std::to_string(i)});
        kvs_payload kvs;
        kvs.key = std::to_string(i);
        kvs.value = std::to_string(i);

        kvs.txn_timestamp = std::chrono::system_clock::to_time_t(
                        std::chrono::system_clock::now());
        instance->put(&kvs);
    }

    // Verify
    int num_found = 0;
    std::vector<std::string> failed_keys; 
    for(const auto& [key, value] : test_map) {
        kvs_payload payload = instance->get(key);
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

    std::cout << "Num levels at end: " << instance->index.levels.size() << "\n";
    for (int i = 0; i < instance->index.levels.size(); i++) {
        std::cout << "Stats for level " << instance->index.levels[i].index << std::endl;
        std::cout << "First key l" << i << ": " << instance->index.levels[i].recordHashes[0].minKey << "\n";
        std::cout << "Last key l" << i << ": " << instance->index.levels[i].recordHashes[instance->index.levels[i].recordHashes.size() - 1].maxKey << "\n";
        std::cout << "Max size l" << i << ": " << instance->index.levels[i].maxSize << "\n";
        std::cout << "Num blocks in l" << i << ": "  << instance->index.levels[i].numBlocks << " and length of actual vector: " << instance->index.levels[i].recordHashes.size() << std::endl;
    }
    std::cout << "no.of.keys not found is: " << test_count << "\n";
    std::cout << "no.of.keys found is: " << num_found << "\n";
    std::cout << "size of test_map: " <<test_map.size() << "\n"; 
    for(int x=0; x < failed_keys.size();x++)
        std::cout << failed_keys.at(x)<<" ";
}
