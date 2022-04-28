#ifndef CAPSULEDB_DRIVER
#define CAPSULEDB_DRIVER

#include <string>
#include "src/shared/capsule.h"
#include "asylo/client.h"

class CapsuleDB_Driver {
    private:
        asylo::EnclaveManager *manager;
        char * kEnclaveName;

    public:        
        CapsuleDB_Driver(std::string enclave_path, char * kEnclaveName, size_t memtable_size);

        void put(const kvs_payload *payload);
        kvs_payload get(const std::string &key);
        void finalize();
};


#endif