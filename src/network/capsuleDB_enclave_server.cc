// Enclave CapsuleDB Server

#include <cstddef>
#include <zmq.hpp>
#include "src/core/engine.hh"
#include "src/enclave/capsuleDB_driver.hh"
#include "src/enclave/capsuleDBRequest.pb.h"
#include <string>
#include <iostream>
#ifndef _WIN32
#include <unistd.h>
#else
#include <windows.h>

#define sleep(n)	Sleep(n)
#endif

#include "absl/flags/flag.h"
#include "absl/flags/parse.h"

ABSL_FLAG(std::string, enclave_path, "", "Path to enclave binary image to load");

size_t MEMTABLE_SIZE = 50;

int main (int argc, char *argv[]) {
    absl::ParseCommandLine(argc, argv);
    const std::string enclave_path = absl::GetFlag(FLAGS_enclave_path);
    LOG_IF(QFATAL, enclave_path.empty()) << "Empty --enclave_path flag.";
    
    std::cout << "Starting enclave server" << std::endl;
    //  Prepare our context and socket
    zmq::context_t context (2);
    zmq::socket_t socket (context, zmq::socket_type::dealer);
    socket.bind ("tcp://*:5555");

    CapsuleDB_Driver db = CapsuleDB_Driver(enclave_path, "Test Enclave", 50);

    std::cout << "Enclave server ready" << std::endl;

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

            db.put(&payload);
        } else {
            std::cout << "Received get request" << std::endl;

            std::string requestedKey = parsedRequest.requestedkey();
            kvs_payload retrievedPayload = db.get(requestedKey);
            
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

            // Push out on socket to send reply back to client
            zmq::message_t reply (outgoing_payload.length());
            memcpy (reply.data (), outgoing_payload.c_str(), outgoing_payload.length());
            socket.send (reply);
        }
        
    }
    // Required for correct enclave behavior
    db.finalize();

    return 0;
}