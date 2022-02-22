#include "cdb_test.hh"
#include "benchmark.h"

// CapsuleDBTestClient client;
const absl::string_view signing_key_pem = {
            R"pem(-----BEGIN EC PRIVATE KEY-----
MHcCAQEEIF0Z0yrz9NNVFQU1754rHRJs+Qt04mr3vEgNok8uyU8QoAoGCCqGSM49
AwEHoUQDQgAE2M/ETD1FV9EFzZBB1+emBFJuB1eh2/XyY3ZdNrT8lq7FQ0Z6ENdm
oG+ldQH94d6FPkRWOMwY+ppB+SQ8XnUFRA==
-----END EC PRIVATE KEY-----)pem"
};

// Wrappers for benchmark
// void benchmark_put(const std::string &key, const std::string &value) {
//     client.put(key, value);
// }

// std::string benchmark_get(const std::string &key) {
//     return client.get(key);
// }

/* 
 * Runs a simple test of CapsuleDB connected to ZMQ client, making use of the current multicast tree implementation.
 * Requests -> coordinator -> (multiple) workers
 */
int run_cdb_test_client() {
    // TODO: Add benchmark here
    // benchmark();
    // benchmark_put("testkey", "testvalue");

    std::unique_ptr <asylo::SigningKey> signing_key(std::move(asylo::EcdsaP256Sha256SigningKey::CreateFromPem(
                                            signing_key_pem)).ValueOrDie());

    asylo::CleansingVector<uint8_t> serialized_signing_key;
    ASSIGN_OR_RETURN(serialized_signing_key,
                            signing_key->SerializeToDer());

    CapsuleDBTestClient client = CapsuleDBTestClient(serialized_signing_key);
    LOG(INFO) << "Done with key setup, returned to run_cdb_test_client";
    while (true) {
        client.put("3945957134849834", "FIRST_VAL");
        client.put("3945957134849835", "SECOND_VAL");
        // benchmark_put("3945957134849834", "FIRST_VAL");
        // benchmark_put("3945957134849835", "SECOND_VAL");
        sleep(3);
        LOG(INFO) << "Get result: " << client.get("3945957134849835");
        LOG(INFO) << "Get result: " << client.get("3945957134849834");
        // LOG(INFO) << "Get result: " << benchmark_get("3945957134849835");
        // LOG(INFO) << "Get result: " << benchmark_get("3945957134849834");
        sleep(5);
    }

    sleep(1 * 1000 * 1000);
    return 0; 
}

int main() {
    run_cdb_test_client();
}