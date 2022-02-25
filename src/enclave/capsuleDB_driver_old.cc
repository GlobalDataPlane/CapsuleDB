/*
 *
 * Copyright 2018 Asylo authors
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include <iostream>
#include <string>

#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "asylo/client.h"
#include "asylo/enclave.pb.h"
#include "asylo/util/logging.h"
#include "asylo/platform/primitives/sgx/loader.pb.h"

#include "src/capsuleDBcpp/capsuleDB.pb.h"
#include "../kvs_include/capsule.h"

ABSL_FLAG(std::string, enclave_path, "", "Path to enclave to load");
ABSL_FLAG(int32_t, blocksize, 5, "The number of pairs per block");

// void SetEnclaveCapsuleDBConfig(asylo::EnclaveLoadConfig *enclave_config, int32_t blocksize) {
//     capsuleDB::CapsuleDBConfig *user_config = enclave_config->MutableExtension(capsuleDB::dbConfig);
//     user_input->set_blocksize(blocksize);
// }

// Populates |enclave_input|->value() with |user_message|.
void SetEnclavePayload(asylo::EnclaveInput *enclave_input,
                           kvs_payload user_payload) {
    capsuleDB::DBRequest *user_input = enclave_input->MutableExtension(capsuleDB::capsuleDBEnclaveInput);
    capsuleDB::DBRequest::kvs_payload new_payload = user_input->payload();
    new_payload.set_key(user_payload.key);
    new_payload.set_value(user_payload.value);
    new_payload.set_timestamp(user_payload.txn_timestamp);
}

void SetEnclaveRequest(asylo::EnclaveInput *enclave_input,
                           std::string key) {
    capsuleDB::DBRequest *user_input = enclave_input->MutableExtension(capsuleDB::capsuleDBEnclaveInput);
    user_input->set_requestedkey(key);
}

// Retrieves encrypted message from |output|. Intended to be used by the reader
// for completing the exercise.
kvs_payload GetEnclaveOutputMessage(const asylo::EnclaveOutput &output) {
    const capsuleDB::DBRequest::kvs_payload& incoming_payload = output.GetExtension(capsuleDB::capsuleDBEnclaveOutput).payload();
    kvs_payload kvs;
    kvs.key = incoming_payload.key();
    kvs.value = incoming_payload.value();
    kvs.txn_timestamp = incoming_payload.timestamp();
    return kvs;
}

int main(int argc, char *argv[]) {
    absl::ParseCommandLine(argc, argv);

    LOG_IF(QFATAL, absl::GetFlag(FLAGS_blocksize) == 0)
        << "Must specify blocksize";

    // Part 1: Initialization

    asylo::EnclaveManager::Configure(asylo::EnclaveManagerOptions());
    auto manager_result = asylo::EnclaveManager::Instance();
    LOG_IF(QFATAL, !manager_result.ok()) << "Could not obtain EnclaveManager";

    // Create an EnclaveLoadConfig object.
    asylo::EnclaveLoadConfig load_config;
    load_config.set_name("capsuleDB_enclave");

    // Create an SgxLoadConfig object.
    asylo::SgxLoadConfig sgx_config;
    asylo::SgxLoadConfig::FileEnclaveConfig file_enclave_config;
    file_enclave_config.set_enclave_path(absl::GetFlag(FLAGS_enclave_path));
    *sgx_config.mutable_file_enclave_config() = file_enclave_config;
    sgx_config.set_debug(true);

    // Set an SGX message extension to load_config.
    *load_config.MutableExtension(asylo::sgx_load_config) = sgx_config;

    // SetEnclaveCapsuleDBConfig(load_config, absl::GetFlag(FLAGS_blocksize));

    asylo::EnclaveManager *manager = manager_result.value();
    asylo::Status status = manager->LoadEnclave(load_config);
    LOG_IF(QFATAL, !status.ok()) << "LoadEnclave failed with: " << status;

    // Part 2: Secure execution

    asylo::EnclaveClient *client = manager->GetClient("demo_enclave");
    asylo::EnclaveInput input;
    asylo::EnclaveOutput output;

    kvs_payload kvs;
    kvs.key = 1;
    kvs.value = 1;
    kvs.txn_timestamp = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

    SetEnclavePayload(&input, kvs);
    status = client->EnterAndRun(input, &output);
    LOG_IF(QFATAL, !status.ok()) << "EnterAndRun failed with: " << status;
    std::cout << "Encrypted message1 from driver:" << std::endl
              << GetEnclaveOutputMessage(output).key << std::endl;

    kvs_payload retrieved = GetEnclaveOutputMessage(output);
    std::cout << "Key Retrieved: " << retrieved.key << " Value Retrieved: " << retrieved.value << "\n"; 


    // Part 3: Finalization

    asylo::EnclaveFinal empty_final_input;
    status = manager->DestroyEnclave(client, empty_final_input);
    LOG_IF(QFATAL, !status.ok()) << "DestroyEnclave failed with: " << status;

    return 0;
}





















// /*
//  *
//  * Copyright 2018 Asylo authors
//  *
//  * Licensed under the Apache License, Version 2.0 (the "License");
//  * you may not use this file except in compliance with the License.
//  * You may obtain a copy of the License at
//  *
//  *     http://www.apache.org/licenses/LICENSE-2.0
//  *
//  * Unless required by applicable law or agreed to in writing, software
//  * distributed under the License is distributed on an "AS IS" BASIS,
//  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  * See the License for the specific language governing permissions and
//  * limitations under the License.
//  *
//  */

// #include <iostream>
// #include <string>

// #include "absl/flags/flag.h"
// #include "absl/flags/parse.h"
// #include "asylo/client.h"
// #include "asylo/enclave.pb.h"
// #include "asylo/util/logging.h"
// #include "asylo/platform/primitives/sgx/loader.pb.h"
// #include "capsuleDB.pb.h"

// ABSL_FLAG(std::string, enclave_path, "", "Path to enclave to load");
// ABSL_FLAG(std::string, message1, "", "The first message to encrypt");
// ABSL_FLAG(std::string, message2, "", "The second message to encrypt");
// ABSL_FLAG(std::string, ciphertext, "", "The ciphertext message to decrypt");

// ABSL_FLAG(int32_t, blocksize, 0, "Number of keys per CapsuleDB block");

// // Populates |enclave_input|->value() with |user_message|.
// void SetEnclaveUserMessage(asylo::EnclaveInput *enclave_input,
//                             const std::string &user_message,
//                             guide::asylo::Demo::Action action) {
//     guide::asylo::Demo *user_input =
//         enclave_input->MutableExtension(guide::asylo::quickstart_input);
//     user_input->set_value(user_message);
//     user_input->set_action(action);
// }

// // Retrieves encrypted message from |output|. Intended to be used by the reader
// // for completing the exercise.
// const std::string GetEnclaveOutputMessage(const asylo::EnclaveOutput &output) {
//     return output.GetExtension(guide::asylo::quickstart_output).value();
// }

// int main(int argc, char *argv[]) {
//     absl::ParseCommandLine(argc, argv);

//     LOG_IF(QFATAL, absl::GetFlag(FLAGS_blocksize).empty())
//         << "Must specify blocksize!";

//     // Part 1: Initialization

//     asylo::EnclaveManager::Configure(asylo::EnclaveManagerOptions());
//     auto manager_result = asylo::EnclaveManager::Instance();
//     LOG_IF(QFATAL, !manager_result.ok()) << "Could not obtain EnclaveManager";

//     // Create an EnclaveLoadConfig object.
//     asylo::EnclaveLoadConfig load_config;
//     load_config.set_name("capsuleDB_enclave");

//     // Create an SgxLoadConfig object.
//     asylo::SgxLoadConfig sgx_config;
//     asylo::SgxLoadConfig::FileEnclaveConfig file_enclave_config;
//     file_enclave_config.set_enclave_path(absl::GetFlag(FLAGS_enclave_path));
//     *sgx_config.mutable_file_enclave_config() = file_enclave_config;
//     sgx_config.set_debug(true);

//     // Set an SGX message extension to load_config.
//     *load_config.MutableExtension(asylo::sgx_load_config) = sgx_config;
//     capsuleDB::CapsuleDBConfig *config = *load_config.MutableExtension(capsuleDB::dbconfig);
//     config->set_blocksize(absl::GetFlag(FLAGS_blocksize));

//     asylo::EnclaveManager *manager = manager_result.value();
//     asylo::Status status = manager->LoadEnclave(load_config);
//     LOG_IF(QFATAL, !status.ok()) << "LoadEnclave failed with: " << status;

//     // Part 2: Secure execution

//     asylo::EnclaveClient *client = manager->GetClient("capsuleDB_enclave");
//     asylo::EnclaveInput input;
//     asylo::EnclaveOutput output;

//     // TODO: Take in request and put here based on CapsulePDU

//     if (!absl::GetFlag(FLAGS_ciphertext).empty()) {
//         SetEnclaveUserMessage(&input, absl::GetFlag(FLAGS_ciphertext),
//                             guide::asylo::Demo::DECRYPT);
//         status = client->EnterAndRun(input, &output);
//         LOG_IF(QFATAL, !status.ok()) << "EnterAndRun failed with: " << status;
//         std::cout << "Decrypted ciphertext from driver:" << std::endl
//                 << GetEnclaveOutputMessage(output) << std::endl;
//     }

//     // Part 3: Finalization

//     asylo::EnclaveFinal empty_final_input;
//     status = manager->DestroyEnclave(client, empty_final_input);
//     LOG_IF(QFATAL, !status.ok()) << "DestroyEnclave failed with: " << status;

//     return 0;
// }
