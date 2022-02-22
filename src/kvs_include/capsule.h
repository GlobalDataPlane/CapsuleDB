#ifndef _CAPSULE_H_
#define _CAPSULE_H_

#include <string>
#include <vector>
#include "../common.h"

typedef struct {
    std::string key;
    std::string value;
    int64_t txn_timestamp;
    std::string txn_msgType;
} kvs_payload;

typedef struct{
    
    std::vector<kvs_payload> payload_l;
    std::string payload_in_transit;
    std::string signature;
    int sender;
    
    std::string prevHash; //Hash ptr to the previous record, not needed for the minimal prototype
    std::string hash;

    int64_t timestamp;
    std::string msgType;

    std::string retAddr;

} capsule_pdu;

#define DUMP_PAYLOAD(payload) LOGI << "Payload Key: " << payload->key << ", Value: " << payload->value << ", Timestamp: " << (int64_t) payload->txn_timestamp << ", MsgType: " << payload->txn_msgType;

#define DUMP_PAYLOAD_LIST(payload_l) for (int i = 0; i < payload_l->size(); i++) { LOG(INFO) << "payload_l - Payload Key: " << ((*payload_l))[i].key << ", Value: " << ((*payload_l))[i].value << ", Timestamp: " << (int64_t) ((*payload_l))[i].txn_timestamp << ", MsgType: " << ((*payload_l))[i].txn_msgType;}

#define DUMP_CAPSULE(dc) {LOG(INFO) << "DC Sender: "<< dc->sender << ", Timestamp: " << (int64_t) dc->timestamp << ", hash: " << dc->hash  << ", prevHash: " << dc->prevHash << ", signature: " << dc->signature << " payload_in_transit: " << dc->payload_in_transit << " message type: " << dc->msgType << " ret addr: " << dc->retAddr; DUMP_PAYLOAD_LIST((&(dc->payload_l)));}

#endif 
