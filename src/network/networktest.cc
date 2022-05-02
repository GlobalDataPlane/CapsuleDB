//
//  Hello World client in C++
//  Connects REQ socket to tcp://localhost:5555
//  Sends "Hello" to server, expects "World" back
//
#include <zmq.hpp>
#include <string>
#include <iostream>
#include <chrono>
#include "src/shared/capsule.h"
#include "src/enclave/capsuleDBRequest.pb.h"


int main ()
{
    //  Prepare our context and socket
    zmq::context_t context (1);
    zmq::socket_t socket (context, zmq::socket_type::dealer);

    std::cout << "Connecting to hello world server..." << std::endl;
    socket.connect ("tcp://localhost:5555");
    std::cout << "Connected" << std::endl;

    // Create test payload
    kvs_payload test_payload;
    test_payload.key = "Test key";
    test_payload.value = "Test value";
    test_payload.txn_timestamp = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

    capsuleDBProtos::DBRequest user_input;

    capsuleDBProtos::Kvs_payload* kvs_payload_serialized = user_input.mutable_payload();
    kvs_payload_serialized->set_key(test_payload.key);
    kvs_payload_serialized->set_value(test_payload.value);
    kvs_payload_serialized->set_txn_timestamp(test_payload.txn_timestamp);
    kvs_payload_serialized->set_txn_msgtype("PUT");

    std::string payload_to_send;
    bool success = user_input.SerializeToString(&payload_to_send);

    std::cout << "Sending payload" << std::endl;
    zmq::message_t msg(payload_to_send.size());
    memcpy(msg.data(), payload_to_send.c_str(), payload_to_send.size());
    socket.send (msg);



    std::cout << "Start Get" << std::endl;
    capsuleDBProtos::DBRequest get_request;
    get_request.set_requestedkey("Test key");
    
    std::string get_to_send;
    success = get_request.SerializeToString(&get_to_send);

    std::cout << "Sending get request" << std::endl;
    zmq::message_t get_outgoing(get_to_send.size());
    memcpy(get_outgoing.data(), get_to_send.c_str(), get_to_send.size());
    socket.send (get_outgoing);


    // Retrive via ZMQ
    std::cout << "Receiving get request" << std::endl;
    zmq::message_t request;
    socket.recv (&request);
    std::string incoming_message = std::string(static_cast<const char*>(request.data()), request.size());

    capsuleDBProtos::DBRequest output_data;

    success = output_data.ParseFromString(incoming_message);
    if (!success) {
        std::cerr << "Failed to deserialize incoming result" << std::endl;
        throw std::logic_error("Request deserialization failure.\n");
    }

    kvs_payload retrieved_payload;
    retrieved_payload.key = output_data.payload().key();
    retrieved_payload.value = output_data.payload().value();
    retrieved_payload.txn_timestamp = output_data.payload().txn_timestamp();
    retrieved_payload.txn_msgType = output_data.payload().txn_msgtype();

    std::cout << "Retrieved value: " << retrieved_payload.value << std::endl;

    return 0;
}

