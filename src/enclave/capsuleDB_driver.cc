/*
 *
 * CapsuleDB enclave interface.  
 *
 */

#include <iostream>
#include <string>
#include <chrono>

#include "capsuleDB_driver.hh"
#include "asylo/client.h"
#include "src/enclave/capsuleDBRequest.pb.h"
#include "src/enclave/capsuleDBSetup.pb.h"
#include "asylo/util/logging.h"
#include "asylo/platform/primitives/sgx/loader.pb.h"

CapsuleDB_Driver::CapsuleDB_Driver(std::string enclave_path, char * kEnclaveName, size_t memtable_size) {
    this->kEnclaveName = kEnclaveName;

    // Part 1: Initialization

    // Prepare |EnclaveManager| with default |EnclaveManagerOptions|
    asylo::EnclaveManager::Configure(asylo::EnclaveManagerOptions());
    auto manager_result = asylo::EnclaveManager::Instance();
    LOG_IF(QFATAL, !manager_result.ok()) << "Could not obtain EnclaveManager";

    // Prepare |load_config| message.
    asylo::EnclaveLoadConfig load_config;
    load_config.set_name(kEnclaveName);

    asylo::EnclaveConfig *config = load_config.mutable_config();
    config->SetExtension(capsuleDBProtos::capsuledb_size, memtable_size);

    // Prepare |sgx_config| message.
    auto sgx_config = load_config.MutableExtension(asylo::sgx_load_config);
    sgx_config->set_debug(true);
    auto file_enclave_config = sgx_config->mutable_file_enclave_config();
    file_enclave_config->set_enclave_path(enclave_path);

    // Load Enclave with prepared |EnclaveManager| and |load_config| message.
    manager = manager_result.value();
    auto status = manager->LoadEnclave(load_config);
    LOG_IF(QFATAL, !status.ok()) << "LoadEnclave failed with: " << status;
}

void CapsuleDB_Driver::put(const kvs_payload *payload) {
    asylo::EnclaveInput input;
    asylo::EnclaveOutput output;
    
    capsuleDBProtos::DBRequest *user_input = input.MutableExtension(capsuleDBProtos::request_input);
    user_input->set_requestingenclaveid(25);

    capsuleDBProtos::Kvs_payload* kvs_payload_serialized = user_input->mutable_payload();
    kvs_payload_serialized->set_key(payload->key);
    kvs_payload_serialized->set_value(payload->value);
    kvs_payload_serialized->set_txn_timestamp(payload->txn_timestamp);
    kvs_payload_serialized->set_txn_msgtype("PUT");

    asylo::EnclaveClient *const client = manager->GetClient(kEnclaveName);
    auto status = client->EnterAndRun(input, &output);
    LOG_IF(QFATAL, !status.ok()) << "EnterAndRun failed with: " << status;
}

kvs_payload CapsuleDB_Driver::get(const std::string &key) {
    asylo::EnclaveInput input;
    asylo::EnclaveOutput output;
    
    capsuleDBProtos::DBRequest *user_input = input.MutableExtension(capsuleDBProtos::request_input);
    user_input->set_requestingenclaveid(25);
    user_input->set_requestedkey(key);
    
    asylo::EnclaveClient *const client = manager->GetClient(kEnclaveName);
    auto status = client->EnterAndRun(input, &output);
    LOG_IF(QFATAL, !status.ok()) << "EnterAndRun failed with: " << status;
    capsuleDBProtos::Kvs_payload output_data = output.GetExtension(capsuleDBProtos::request_output).payload();

    kvs_payload retrieved_payload;
    retrieved_payload.key = output_data.key();
    retrieved_payload.value = output_data.value();
    retrieved_payload.txn_timestamp = output_data.txn_timestamp();
    retrieved_payload.txn_msgType = output_data.txn_msgtype();

    return retrieved_payload;
}

void CapsuleDB_Driver::finalize() {
    // |DestroyEnclave| before exiting program.
    asylo::EnclaveFinal empty_final_input;
    asylo::EnclaveClient *const client = manager->GetClient(kEnclaveName);
    auto status = manager->DestroyEnclave(client, empty_final_input);
    LOG_IF(QFATAL, !status.ok()) << "DestroyEnclave failed with: " << status;
}

