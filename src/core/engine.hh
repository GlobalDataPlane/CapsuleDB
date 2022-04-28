#ifndef ENGINE_H
#define ENGINE_H

/*
 * This file manages the database as well as read/write requests.  
 */
#include <iostream>
#include <string>
#include <vector>

#include "index.hh"
#include "memtable.hpp"
#include "src/shared/capsule.h"


class CapsuleDB {
    public:
        std::string name;
        int maxLevels = 5;
        std::string targetCapsule;
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
        
        CapsuleDB() {}
        kvs_payload get(const std::string &key, bool isMulticast = false);
        void put(const kvs_payload *payload);
};

/*
 * This function creates a new CapsuleDB instance.  It takes in information about the sizes of levels and other metadata to establish compaction rules.
 * 
 * Inputs: Size of the memtable (and by extension the max size of each CapsuleBlock)
 * Outputs: An error code
 */
CapsuleDB* spawnDB(size_t memtable_size);
int connectDB();

#endif
