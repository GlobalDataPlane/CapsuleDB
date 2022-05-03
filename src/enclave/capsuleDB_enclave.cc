#include <string>

#include "asylo/trusted_application.h"
#include "asylo/util/status_macros.h"

#include "../core/engine.hh"
#include "src/shared/capsule.h"
#include "src/enclave/capsuleDBRequest.pb.h"
#include "src/enclave/capsuleDBSetup.pb.h"

CapsuleDB * db;

namespace asylo {

class CapsuleDBEnclaveClient : public TrustedApplication {
    public:
    CapsuleDBEnclaveClient() = default;

    Status Initialize(const asylo::EnclaveConfig &enclave_config) {
        if (enclave_config.HasExtension(capsuleDBProtos::capsuledb_size)) {
            db = spawnDB(enclave_config.GetExtension(capsuleDBProtos::capsuledb_size));
        } else {
            db = spawnDB(50);
        }
        return absl::OkStatus();
    }

    Status Run(const EnclaveInput &input, EnclaveOutput *output) {
        
        if (input.GetExtension(capsuleDBProtos::request_input).requestedkey() == "") {
            kvs_payload payload;
            payload.key = input.GetExtension(capsuleDBProtos::request_input).payload().key();
            payload.value = input.GetExtension(capsuleDBProtos::request_input).payload().value();
            payload.txn_timestamp = input.GetExtension(capsuleDBProtos::request_input).payload().txn_timestamp();
            db->put(&payload);
        } else {
            std::string requestedKey = input.GetExtension(capsuleDBProtos::request_input).requestedkey();
            kvs_payload retrievedPayload = db->get(requestedKey);
            capsuleDBProtos::DBRequest *output_request = output->MutableExtension(capsuleDBProtos::request_output);
            capsuleDBProtos::Kvs_payload* kvs_payload_serialized = output_request->mutable_payload();
            kvs_payload_serialized->set_key(retrievedPayload.key);
            kvs_payload_serialized->set_value(retrievedPayload.value);
            kvs_payload_serialized->set_txn_timestamp(retrievedPayload.txn_timestamp);
        }
        return absl::OkStatus();
    }
};

TrustedApplication *BuildTrustedApplication() { return new CapsuleDBEnclaveClient; }

}  // namespace asylos
