// CapsuleDB Server

#include <cstddef>
#include <zmq.hpp>
#include "src/core/engine.hh"
#include "src/enclave/capsuleDBRequest.pb.h"
#include <string>
#include <iostream>
#ifndef _WIN32
#include <unistd.h>
#else
#include <windows.h>

#define sleep(n)	Sleep(n)
#endif

size_t MEMTABLE_SIZE = 50;

int main () {
    
    std::cout << "Starting server" << std::endl;
    //  Prepare our context and socket
    zmq::context_t context (2);
    zmq::socket_t socket (context, zmq::socket_type::dealer);
    socket.bind ("tcp://*:5555");

    CapsuleDB *db = spawnDB(MEMTABLE_SIZE);
    // zmq::socket_t* mcast_socket = new zmq::socket_t(context, ZMQ_PUSH);
    // zmq::socket_t* recv_socket = new zmq::socket_t(context, ZMQ_PULL);

    std::cout << "Server ready" << std::endl;

    while (true) {
        zmq::message_t request;

        //  Wait for next request from client
        socket.recv (&request);

        std::cout << "Received new message" << std::endl;

        std::string incoming_message = std::string(static_cast<const char*>(request.data()), request.size());

        capsuleDBProtos::DBRequest parsedRequest;
        bool success = parsedRequest.ParseFromString(incoming_message);
        if (!success) {
            throw std::logic_error("Failed to deserialize incoming request.\n");
        }

        if (parsedRequest.requestedkey() == "") {
            std::cout << "Received put request" << std::endl;

            kvs_payload payload;
            
            payload.key = parsedRequest.payload().key();
            payload.value = parsedRequest.payload().value();
            payload.txn_timestamp = parsedRequest.payload().txn_timestamp();
            payload.txn_msgType = parsedRequest.payload().txn_msgtype();
            
            std::cout << "Payload key: " << payload.key << " and value: " << payload.value << std::endl;

            db->put(&payload);

            zmq::message_t reply (5);
            memcpy (reply.data (), "World", 5);
            socket.send (reply, 0);

        } else {
            std::cout << "Received get request" << std::endl;

            std::string requestedKey = parsedRequest.requestedkey();
            kvs_payload retrievedPayload = db->get(requestedKey);
            
            capsuleDBProtos::DBRequest output_request;
            capsuleDBProtos::Kvs_payload* kvs_payload_serialized = output_request.mutable_payload();
            

            kvs_payload_serialized->set_key(retrievedPayload.key);
            kvs_payload_serialized->set_value(retrievedPayload.value);
            kvs_payload_serialized->set_txn_timestamp(retrievedPayload.txn_timestamp);
            kvs_payload_serialized->set_txn_msgtype(retrievedPayload.txn_msgType);

            std::string outgoing_payload;
            success = output_request.SerializeToString(&outgoing_payload);
            if (!success) {
                std::cerr << "Failed to serialize outgoing request " << std::endl;
                throw std::logic_error("Request serialization failure.\n");
            }

            // Push out on zmq
            //  Send reply back to client
            zmq::message_t reply (outgoing_payload.length());
            memcpy (reply.data (), outgoing_payload.c_str(), outgoing_payload.length());
            socket.send (reply);
        }
        
    }
    return 0;
}