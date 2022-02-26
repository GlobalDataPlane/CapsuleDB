#include <string>
#include <vector>
#include "engine.hh"
#include "../kvs_include/capsule.h"

// Asylo
#include "absl/base/macros.h"
#include "absl/status/status.h"
#include "asylo/crypto/util/byte_container_view.h"
#include "asylo/trusted_application.h"
#include "asylo/util/cleansing_types.h"
#include "asylo/util/status_macros.h"
#include "asylo/util/statusor.h"
// #include "src/proto/hello.pb.h"
// #include "src/util/proto_util.hpp"

namespace asylo {
    namespace {

    } // namespace

    class CapsuleDBClient : public TrustedApplication {
        public:
            CapsuleDBClient() = default;

            // Asylo enclave management functions
            Status Initialize(const EnclaveConfig &config);
            Status Run(const asylo::EnclaveInput &input, asylo::EnclaveOutput *output);

        private:
            // int m_enclave_id;
            // int64_t m_lamport_timer;
            
            // std::string priv_key;
            // std::string pub_key;
            // std::unique_ptr <SigningKey> signing_key;
            // std::unique_ptr <VerifyingKey> verifying_key;
            // HotMsg *buffer;
            // int requestedCallID;
            // int counter;
            // std::string m_prev_hash;
            // std::unordered_map<int, std::pair<std::string, int64_t>> m_eoe_hashes;
            // PQueue pqueue;
            
            CapsuleDB db;

            // void handle()
            // std::string serialize_eoe_hashes();
            // void compare_eoe_hashes_from_string(std::string s);
            // void update_client_hash(capsule_pdu* dc);
            // void inconsistency_handler();
            // void put_ocall(capsule_pdu *dc);

            // int HotMsg_requestOCall( HotMsg* hotMsg, int dataID, void *data );
            // void EnclaveMsgStartResponder( HotMsg *hotMsg );

            int32_t GetEnclaveBlocksize(const EnclaveConfig &config);
            const std::string GetEnclaveRequestedKey(const EnclaveInput &input);
            kvs_payload GetEnclavePayload(const EnclaveInput &input);
            void SetEnclaveOutputPayload(EnclaveOutput *enclave_output, kvs_payload payload);
    };


} // namespace asylo



