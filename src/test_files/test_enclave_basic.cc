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
#include <chrono>

#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "asylo/client.h"
#include "src/enclave/capsuleDBRequest.pb.h"
#include "src/enclave/capsuleDBSetup.pb.h"
#include "asylo/util/logging.h"
#include "asylo/platform/primitives/sgx/loader.pb.h"

ABSL_FLAG(std::string, enclave_path, "",
          "Path to enclave binary image to load");

// Populates |enclave_input|->value() with |user_message|.
void SetEnclaveUserMessage(asylo::EnclaveInput *enclave_input,
                           bool get) {
  capsuleDBProtos::DBRequest *user_input =
      enclave_input->MutableExtension(capsuleDBProtos::request_input);
  user_input->set_requestingenclaveid(25);
  if (get) {
    user_input->set_requestedkey("TestKey");
  } else {
    capsuleDBProtos::Kvs_payload* kvs_payload_serialized = user_input->mutable_payload();
    kvs_payload_serialized->set_key("TestKey");
    kvs_payload_serialized->set_value("TestVal");
    kvs_payload_serialized->set_txn_timestamp(std::chrono::system_clock::to_time_t(
                            std::chrono::system_clock::now()));
    kvs_payload_serialized->set_txn_msgtype("PUT");
  }
}

int main(int argc, char *argv[]) {
  absl::ParseCommandLine(argc, argv);

  constexpr char kEnclaveName[] = "capsuleDB_enclave";

  const std::string enclave_path = absl::GetFlag(FLAGS_enclave_path);
  LOG_IF(QFATAL, enclave_path.empty()) << "Empty --enclave_path flag.";

  

  // Part 1: Initialization

  // Prepare |EnclaveManager| with default |EnclaveManagerOptions|
  asylo::EnclaveManager::Configure(asylo::EnclaveManagerOptions());
  auto manager_result = asylo::EnclaveManager::Instance();
  LOG_IF(QFATAL, !manager_result.ok()) << "Could not obtain EnclaveManager";

  // Prepare |load_config| message.
  asylo::EnclaveLoadConfig load_config;
  load_config.set_name(kEnclaveName);

  asylo::EnclaveConfig *config = load_config.mutable_config();
  config->SetExtension(capsuleDBProtos::capsuledb_size, 50);

  // Prepare |sgx_config| message.
  auto sgx_config = load_config.MutableExtension(asylo::sgx_load_config);
  sgx_config->set_debug(true);
  auto file_enclave_config = sgx_config->mutable_file_enclave_config();
  file_enclave_config->set_enclave_path(enclave_path);

  // Load Enclave with prepared |EnclaveManager| and |load_config| message.
  asylo::EnclaveManager *manager = manager_result.value();
  auto status = manager->LoadEnclave(load_config);
  LOG_IF(QFATAL, !status.ok()) << "LoadEnclave failed with: " << status;

  // Part 2: Secure execution

  // Prepare |input| with |message| and create |output| to retrieve response
  // from enclave.
  asylo::EnclaveInput input;
  SetEnclaveUserMessage(&input, false);
  asylo::EnclaveOutput output;

  // Get |EnclaveClient| for loaded enclave and execute |EnterAndRun|.
  asylo::EnclaveClient *const client = manager->GetClient(kEnclaveName);
  status = client->EnterAndRun(input, &output);
  LOG_IF(QFATAL, !status.ok()) << "EnterAndRun failed with: " << status;

  SetEnclaveUserMessage(&input, true);
  status = client->EnterAndRun(input, &output);
  LOG_IF(QFATAL, !status.ok()) << "EnterAndRun failed with: " << status;
  std::cout << "Retrieved Value: " << output.GetExtension(capsuleDBProtos::request_output).payload().value() << std::endl;

  // Part 3: Finalization

  // |DestroyEnclave| before exiting program.
  asylo::EnclaveFinal empty_final_input;
  status = manager->DestroyEnclave(client, empty_final_input);
  LOG_IF(QFATAL, !status.ok()) << "DestroyEnclave failed with: " << status;

  return 0;
}
