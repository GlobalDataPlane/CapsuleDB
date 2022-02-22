#ifndef CAPSULEBLOCK_H
#define CAPSULEBLOCK_H

#include <string>
#include <vector>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/vector.hpp>
#include "../kvs_include/capsule.h"

typedef struct {
    std::string hash;
    std::string minKey;
    std::string maxKey;
} blockHeader;

namespace boost{
    namespace serialization{
        template<class Archive>
        void serialize (Archive & ar, kvs_payload & g, const unsigned int version){
            ar & g.key;
            ar & g.value;
            ar & g.txn_timestamp;
            ar & g.txn_msgType;
        }
    }
}

class CapsuleBlock {
    private:
        friend class boost::serialization::access;
        template<class Archive>
        void serialize(Archive & ar, const unsigned int version) {
            ar & level;
            ar & startKey;
            ar & endKey;
            ar & kvPairs;
        }

    public:
        int level;
        std::string startKey; // Defines the range of keys contained in this block
        std::string endKey;
        std::vector<kvs_payload> kvPairs; // Key, value, timestamp, msgType
        
        CapsuleBlock();
        CapsuleBlock(int l);
        int getLevel();
        std::string getMinKey();
        std::string getMaxKey();
        std::vector<kvs_payload> getKVPairs();
        void addKVPair(kvs_payload);
        void setMinKey(std::string k);
        void setMaxKey(std::string k);
        std::string writeOut();
        void readIn(std::string transactionHash, CapsuleBlock *location);
};

void readIn(std::string transactionHash, CapsuleBlock *location);

#endif