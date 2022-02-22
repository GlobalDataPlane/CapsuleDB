#include <string>
#include <thread>
#include <vector>
#include <zmq.hpp>
#include <memory>

#include "common.h"

#include "zmq_comm.hpp"
#include "capsuleDBcpp/cdb_network_client.hh"
#include "kvs_include/capsule.h"

class CapsuleDBTestClient {
    private:
        std::string recv_addr;

        zmq::context_t context_ = zmq::context_t(1);
        zmq::socket_t* mcast_socket = new zmq::socket_t(context_, ZMQ_PUSH);
        zmq::socket_t* recv_socket = new zmq::socket_t(context_, ZMQ_PULL);

        std::unique_ptr <asylo::SigningKey> signing_key;
        std::unique_ptr <asylo::VerifyingKey> verifying_key;

        // Yeah :)
        // Converts socket message to CapsulePDU
        capsule_pdu recv_capsule_pdu(zmq::socket_t* socket) {
            zmq::message_t message;
            socket->recv(&message);
            std::string response = std::string(static_cast<const char*>(message.data()), message.size());

            hello_world::CapsulePDU in_dc;
            in_dc.ParseFromString(response);

            capsule_pdu translated;
            asylo::CapsuleFromProto(&translated, &in_dc);
            return translated;
        }

        /* Waits for any response on the recv port. 
         * Called for GET requests.
         */
        std::string wait_response(const std::string &key) {
            LOG(INFO) << "Waiting for response";

            // Vector of sockets to poll from. In this case we are only looking for recv messages.
            std::vector<zmq::pollitem_t> pollitems = {
                    { static_cast<void *>(*recv_socket), 0, ZMQ_POLLIN, 0 },
            };

            // Continuously poll until receive response
            while (true) {
                zmq::poll(pollitems.data(), pollitems.size(), 0);
                if (pollitems[0].revents & ZMQ_POLLIN) {
                    LOG(INFO) << "Got response!!!!";

                    // Convert socket message -> decrypted CapsulePDU
                    capsule_pdu response = recv_capsule_pdu(recv_socket);

                    if(asylo::decrypt_payload_l(&response)) {
                    // Returns value of first PUT kvs_payload received
                    // Note: does not handle request batching
                        for (kvs_payload payload : response.payload_l) {
                            if (payload.txn_msgType == "PUT") {
                                return payload.value;
                            }
                        }
                    }
                    return "";
                }
            }
        }

        zmq::message_t gen_payload(const std::string &key, const std::string &value, bool isPut) {
            kvs_payload payload;
            std::vector<kvs_payload> payload_l;
            int64_t currTime = std::chrono::system_clock::to_time_t(
                                std::chrono::system_clock::now());

            // TODO: Formally define msgType
            if (isPut) {
                asylo::KvToPayload(&payload, key, value, currTime, "PUT");
            } else {
                asylo::KvToPayload(&payload, key, value, currTime, "GET");
            }
            payload_l.push_back(payload);

            // Create CapsulePDU
            auto *dc = new capsule_pdu();
            asylo::PayloadListToCapsule(dc, &payload_l, 0, recv_addr);
            // Encrypt
            asylo::encrypt_payload_l(dc, true);
            // Hash
            asylo::generate_hash(dc);
            // Sign
            bool success = asylo::sign_dc(dc, signing_key);

            DUMP_CAPSULE(dc);

            // Convert to protobuf
            hello_world::CapsulePDU out_dc;
            asylo::CapsuleToProto(dc, &out_dc);

            // Convert to string
            std::string out_s;
            out_dc.SerializeToString(&out_s);

            // Convert to message_t (zmq socket message)
            zmq::message_t msg(out_s.size());
            memcpy(msg.data(), out_s.c_str(), out_s.size());

            return msg;
        }


    public:       
        CapsuleDBTestClient(asylo::CleansingVector<uint8_t> serialized_signing_key) {
            // Connect to the multicast socket of the root router
            std::string coordinator_ip = NET_SEED_ROUTER_IP;
            mcast_socket->connect ("tcp://" + coordinator_ip + ":6667");
            
            // Bind the recv socket to receive GET responses
            recv_socket->bind ("tcp://*:" + std::to_string(NET_CDB_TEST_RESULT_PORT));
            std::string ip = NET_CDB_TEST_CLIENT_IP;
            recv_addr = "tcp://" + ip + ":" + std::to_string(NET_CDB_TEST_RESULT_PORT);
            LOG(INFO) << "Bind recv socket: " << recv_addr;

            // Set up signing and verifying keys
            this->signing_key = asylo::EcdsaP256Sha256SigningKey::CreateFromDer(serialized_signing_key).ValueOrDie();
            this->verifying_key = this->signing_key->GetVerifyingKey().ValueOrDie();
            
            LOG(INFO) << "Finish test client setup!";
        }
        
        // Multicasts a PUT request
        void put(const std::string &key, const std::string &value) {
            zmq::message_t msg = gen_payload(key, value, true);
            mcast_socket->send(msg);
            LOG(INFO) << "Sent put msg!";
        }

        // Multicasts a GET request, awaits response, and returns response value
        std::string get(const std::string &key) {
            zmq::message_t msg = gen_payload(key, "UNUSED_VAL", false);
            mcast_socket->send(msg);
            LOG(INFO) << "Sent get msg!";

            // Waits for reponse
            return wait_response(key);
            // return "";
        }        
};