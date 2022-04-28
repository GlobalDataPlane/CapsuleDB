/*
 * This file defines a simple test for the CapsuleDB enclave interface.
 * It's purpose is to verify that the enclave interface is correctly operating. 
 * For a simple enclave test, see test_enclave_basic.cc
 */

#include <string>
#include <chrono>

#include "src/enclave/capsuleDB_driver.hh"
#include "src/shared/capsule.h"

#include "absl/flags/flag.h"
#include "absl/flags/parse.h"

ABSL_FLAG(std::string, enclave_path, "", "Path to enclave binary image to load");

int main(int argc, char *argv[]) {
    absl::ParseCommandLine(argc, argv);
    const std::string enclave_path = absl::GetFlag(FLAGS_enclave_path);
    LOG_IF(QFATAL, enclave_path.empty()) << "Empty --enclave_path flag.";
    
    // Initialize enclave and CapsuleDB instance
    CapsuleDB_Driver db = CapsuleDB_Driver(enclave_path, "Test Enclave", 50);

    // Create test payload
    kvs_payload test_payload;
    test_payload.key = "Test key";
    test_payload.value = "Test value";
    test_payload.txn_timestamp = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

    // Enters enclave to put value
    db.put(&test_payload);

    // Enters enclave to retrieve value
    kvs_payload retrieved_payload = db.get("Test key");
    std::cout << "Retrieved value: " << retrieved_payload.value << std::endl;

    // Required for correct teardown behavior
    db.finalize();
}