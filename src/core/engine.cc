/*
 * This file manages the database as well as read/write requests.  
 *
 */

#include "src/shared/common.h"
#include "engine.hh"

// using namespace asylo;

/*
 * This function creates a new CapsuleDB instance.  It takes in information about the sizes of levels and other metadata to establish compaction rules.
 * 
 * Inputs: ??? (Maybe name?)
 * Outputs: An error code
 */
CapsuleDB* spawnDB(size_t memtable_size)
{
    CapsuleDB* newInstance = new CapsuleDB();
    newInstance->memtable = Memtable(memtable_size);
    newInstance->index = CapsuleIndex(memtable_size);
    return newInstance;
}

/*
 * This function connects to a CapsuleDB instance.
 * 
 * Input: None
 * Output: An error code
 */
int connectDB()
{
    return 0;
}

/*
 * This function takes in a kvs_payload and writes it to CapsuleDB
 *
 * Input: A kvs payload
 * Output: Nothing
 */
void CapsuleDB::put(const kvs_payload *payload)
{   
    std::cout << "PUT key=" << payload->key << ", value=" << payload->value << "\n";
    if (!memtable.put(payload, &this->index))
    {
        #ifdef DEBUG
        std::cout << "Failed to write key in the Database";
        #endif
    }
}

/* 
 * This function retrieves a key from CapsuleDB.  It queries the DataCapsule for the most recent index and then traverses the DataCapsule to find the requested key.
 * It returns the kvs_payload either directly to the requesting enclave or multicasts it depending on selected mode.
 *
 * Inputs: The key whose value is requested, the requesting enclave, and a return mode.
 * Output: The requested value or an error if the key does not exist.
 */
kvs_payload CapsuleDB::get(const std::string &key, bool isMulticast /* default is already false from function declaration in engine.hh */)
{
    // #ifdef DEBUG
    std::cout << "GET key=" << key << "\n";
    // #endif

    int level_info;
    std::string block_info, k;

    kvs_payload kv = memtable.get(key);
    if (kv.key == "") {
        
        #ifdef DEBUG
        std::cout << "Couldn't find key in Memtable, checking Index...\n";
        #endif
        
        level_info = index.getNumLevels();
        for (int i = 0; i < level_info; i++)
        {
            block_info = index.getBlock(i, key);
            if (block_info != "") // Key might be present, however verify if key exists if not check other levels
            {   
                #ifdef DEBUG
                std::cout << "Checking block " << block_info << "\n";
                #endif

                CapsuleBlock block;
                readIn(block_info, &block);
                for (long unsigned int j = 0; j < block.kvPairs.size(); j++) 
                {
                    
                    kvs_payload kv_tuple = block.kvPairs[j];
                    
                    #ifdef DEBUG
                    std::cout << "CurrKey=" << kv_tuple.key << "\n";
                    std::cout << "CurrValue=" << kv_tuple.key << "\n";
                    #endif

                    if (i != 0 && kv_tuple.key > key) 
                    {
                        break;
                    } else if (kv_tuple.key == key) 
                    {
                        return kv_tuple;
                    } 

                }
            }
        }
    }

    #ifdef DEBUG
    std::cout << "CapsuleDb: Couldn't find key: " << key << "\n";
    #endif

    return kv;
}
