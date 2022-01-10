#include "proto_util.hpp"
#include <unistd.h>
#include "asylo/util/logging.h"
#include "absl/strings/str_split.h"
// TODO: currently we get timestamp by ocall, we need optimization here
#include <sys/time.h>
#include "../crypto.h"

std::string delim_str = "@@@";
char delim = ';';

namespace asylo {

    int64_t get_current_time(){
        struct timeval tp;
        gettimeofday(&tp, NULL);
        return tp.tv_sec * 1000 + tp.tv_usec / 1000;
    }

    std::string serialize_payload_l(const std::vector<kvs_payload> &payload_l) {
        std::string payload_l_s;
        for( const kvs_payload& payload : payload_l ) {
            payload_l_s += std::to_string(payload.txn_timestamp) + delim_str + payload.txn_msgType + delim_str 
                            + payload.key + delim_str + payload.value + delim_str;
        }
        return payload_l_s;
    }

    std::vector<kvs_payload> deserialize_payload_l(const std::string &payload_l_s) {
        std::vector<kvs_payload> payload_l;
        std::stringstream ss(payload_l_s);
        std::string txn_timestamp, txn_msgType, key, value;

        std::vector<std::string> split = absl::StrSplit(payload_l_s, delim_str, absl::SkipEmpty());
        
        if(split.size() % 4 != 0){
            LOG(ERROR) << "invalid payload size " << split.size();
            for(int i = 0; i < split.size(); i+=1) {
                LOG(ERROR) << i << " " << split.at(4);
            }
            return payload_l;
        }

        for (int i=0; i < (split.size() / 4)  * 4; i+=4) {
            kvs_payload payload;
            txn_timestamp = split.at(i);
            txn_msgType = split.at(i + 1);
            key = split.at(i + 2);
            value = split.at(i + 3);
            
            KvToPayload(&payload, key, value, std::stoi(txn_timestamp), txn_msgType);
            payload_l.push_back(payload);
        }
        
        return payload_l;
    }

    bool generate_hash(capsule_pdu *dc){
        const std::string aggregated = std::to_string(dc->sender) + std::to_string(dc->timestamp)
                                        +dc->payload_in_transit;
        std::vector<uint8_t> digest;
        bool success = DoHash(aggregated, &digest);
        if (!success) return false;
        dc->hash = BytesToHexString(digest);
        return true;
    }

    bool sign_dc(capsule_pdu *dc, const std::unique_ptr <SigningKey> &signing_key) {
        std::string aggregated = dc->hash + dc->prevHash;
        dc->signature = SignMessage(aggregated, signing_key);
        return true;
    }

    bool verify_hash(const capsule_pdu *dc){
        const std::string aggregated = std::to_string(dc->sender) + std::to_string(dc->timestamp)
                                        +dc->payload_in_transit;
        std::vector<uint8_t> digest;
        bool success = DoHash(aggregated, &digest);
        if (!success) return false;
        return dc->hash == BytesToHexString(digest);
    }

    bool verify_signature(const capsule_pdu *dc, const std::unique_ptr <VerifyingKey> &verifying_key) {
        return VerifyMessage(dc->hash + dc->prevHash, dc->signature, verifying_key);
    }

    bool verify_dc(const capsule_pdu *dc, const std::unique_ptr <VerifyingKey> &verifying_key){
        
        // verify hash matches
        bool hash_result = verify_hash(dc);
        if (!hash_result) {
            LOGI << "hash verification failed!!!";
            return false;
        }
        // LOG(INFO) << "after verify_hash";
        // verify signature
        bool sig_result = verify_signature(dc, verifying_key);
        if (!sig_result) {
            LOGI << "signature verification failed!!!";
            return false;
        }

        // TODO: verify prevHash matches. Need to clean up m_eoe_hash logic before implementation.
        // if (dc->prevHash == "init") return true; // sender's first pdu
        // auto got = m_eoe_hashes->find(dc->sender);
        // if (got == m_eoe_hashes->end()){
        //     LOGI << "prevHash verification failed!!! expected prevHash not found.";
        //     return false;
        // } else {
        //     bool prev_hash_result = got->second.first == dc->prevHash;
        //     if (!prev_hash_result) {
        //         LOGI << "prevHash verification failed!!!";
        //         LOGI << "expected: " << got->second.first;
        //         LOGI << "received: " << dc->prevHash;
        //         return false;
        //     }
        // }

        return true;
    }

    bool encrypt_payload_l(capsule_pdu *dc) {
        std::string aggregated = serialize_payload_l(dc->payload_l);
        std::string encrypted_aggregated;

        ASSIGN_OR_RETURN_FALSE(encrypted_aggregated, EncryptMessage(aggregated));
        dc->payload_in_transit = encrypted_aggregated;
        return true;
    }

    bool decrypt_payload_l(capsule_pdu *dc) {
        std::string decrypted_aggregated;

        ASSIGN_OR_RETURN_FALSE(decrypted_aggregated, DecryptMessage(dc->payload_in_transit));
        // std::cout << "After DecryptMessage: " << decrypted_aggregated << std::endl;
        // std::cout << std::endl;
        dc->payload_l = deserialize_payload_l(decrypted_aggregated);
        return true;
    }

    void KvToPayload(kvs_payload *payload, const std::string &key, const std::string &value, const int64_t timer,
                    const std::string &msgType) {
        payload->key = key;
        payload->value = value;
        payload->txn_timestamp = timer;
        payload->txn_msgType = msgType;
    }

    void PayloadListToCapsule(capsule_pdu *dc, const std::vector<kvs_payload> *payload_l, const int enclave_id) {
        dc->payload_l = *payload_l;
        dc->timestamp = payload_l->back().txn_timestamp;
        dc->msgType = payload_l->back().txn_msgType;
        dc->sender = enclave_id;
    }

    void CapsuleToProto(const capsule_pdu *dc, hello_world::CapsulePDU *dcProto){

        dcProto->set_payload_in_transit(dc->payload_in_transit);
        dcProto->set_signature(dc->signature);
        dcProto->set_sender(dc->sender);

        dcProto->set_prevhash(dc->prevHash);
        dcProto->set_hash(dc->hash);

        dcProto->set_timestamp(dc->timestamp);
        dcProto->set_msgtype(dc->msgType);

    }

    void CapsuleFromProto(capsule_pdu *dc, const hello_world::CapsulePDU *dcProto) {

        dc->signature = dcProto->signature();
        dc->sender = dcProto->sender();
        dc->payload_in_transit = dcProto->payload_in_transit();

        dc->prevHash = dcProto->prevhash();
        dc->hash = dcProto->hash();

        dc->timestamp = dcProto->timestamp();
        dc->msgType = dcProto->msgtype();
    }

    void CapsuleToCapsule(capsule_pdu *dc_new, const capsule_pdu *dc) {
        dc_new->payload_l = dc->payload_l;

        dc_new->signature = dc->signature;
        dc_new->sender = dc->sender;
        dc_new->payload_in_transit = dc->payload_in_transit;

        dc_new->prevHash = dc->prevHash;
        dc_new->hash = dc->hash;

        dc_new->timestamp = dc->timestamp;
        dc_new->msgType = dc->msgType;
    }

    void dumpProtoCapsule(const hello_world::CapsulePDU *dcProto){
        LOGI << "Sender: "<< dcProto->sender() << ", payload_in_transit: " << dcProto->payload_in_transit() << ", Timestamp: " << (int64_t) dcProto->timestamp()
                  << ", hash: " << dcProto->hash() << ", prevHash: " << dcProto->prevhash()
                  << ", signature: " << dcProto->signature() << " message type: " << dcProto->msgtype();
    }

} // namespace asylo