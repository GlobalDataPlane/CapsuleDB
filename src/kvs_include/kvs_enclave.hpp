#pragma once

#include <cstdint>
#include "absl/base/macros.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/escaping.h"
#include "asylo/trusted_application.h"
#include "asylo/util/logging.h"
#include "asylo/util/status.h"
#include "asylo/crypto/aead_cryptor.h"
#include "asylo/util/cleansing_types.h"
#include "asylo/crypto/ecdsa_p256_sha256_signing_key.h"
#include "asylo/util/status_macros.h"
#include "asylo/crypto/util/byte_container_view.h"
#include "asylo/platform/primitives/trusted_primitives.h"
#include "asylo/identity/platform/sgx/sgx_identity_util.h"
#include "asylo/identity/attestation/sgx/sgx_local_assertion_generator.h"
#include "../kvs_include/capsule.h"
#include "../hot_msg_pass.h"
#include "../common.h"
#include "../duktape/duktape.h"
#include "../pqueue.hpp"
#include "../capsuleDBcpp/memtable_new.hpp"

#include "src/proto/hello.pb.h"
#include "src/util/proto_util.hpp"
//GRPC 
#include "src/translator_server.grpc.pb.h"
#include "asylo/grpc/auth/enclave_channel_credentials.h"
#include "asylo/grpc/auth/sgx_local_credentials_options.h"
#include "include/grpc/support/time.h"
#include "include/grpcpp/grpcpp.h"

#include <utility>
#include <unordered_map>

 
 namespace asylo {
   
   class KVSClient : public asylo::TrustedApplication {
    public:
        KVSClient(){}
        void put(std::string key, std::string value, std::string msgType);
        kvs_payload get(const std::string &key);
        asylo::Status Initialize(const EnclaveConfig &config);
        asylo::Status Run(const asylo::EnclaveInput &input, asylo::EnclaveOutput *output) override;
        void benchmark();
        void handle();

     private:
        MemTable memtable;
        HotMsg *buffer;
        int requestedCallID;
        int counter;
        duk_context *ctx;
        std::string priv_key;
        std::string pub_key;
        bool is_coordinator;
        int m_enclave_id;
        std::string m_prev_hash;
        std::unordered_map<int, std::pair<std::string, int64_t>> m_eoe_hashes;
        int64_t m_lamport_timer;
        PQueue pqueue;
        std::unique_ptr <SigningKey> signing_key;
        std::unique_ptr <VerifyingKey> verifying_key;

        void put_internal(capsule_pdu *dc, bool to_memtable, bool update_hash, bool to_network);
        std::string serialize_eoe_hashes();
        void compare_eoe_hashes_from_string(std::string s);
        void update_client_hash(capsule_pdu* dc);
        void inconsistency_handler();
        void put_ocall(capsule_pdu *dc);
        void duk_init_mem_interface(duk_context *ctx);

        int HotMsg_requestOCall( HotMsg* hotMsg, int dataID, void *data );
        void EnclaveMsgStartResponder( HotMsg *hotMsg );
    };

    namespace {
    
    using examples::grpc_server::Translator;
    using examples::grpc_server::RetrieveKeyPairResponse;
    using examples::grpc_server::RetrieveKeyPairRequest;

    const absl::Duration kChannelDeadline = absl::Seconds(5);

    // Makes a GetKeyPair RPC with |request| to the server backed by *|stub|.
    StatusOr<RetrieveKeyPairResponse> RetrieveKeyPair(
        const RetrieveKeyPairRequest &request, Translator::Stub *stub) {
    RetrieveKeyPairResponse response;

    ::grpc::ClientContext context;
    ASYLO_RETURN_IF_ERROR(
        asylo::Status(stub->RetrieveKeyPair(&context, request, &response)));
    return response;
    }

    static duk_ret_t js_print(duk_context *ctx) {
        std::cout << duk_to_string(ctx, 0) << std::endl;
        return 0;  /* no return value (= undefined) */
    }

    static duk_ret_t js_put(duk_context *ctx){
        std::string key = duk_to_string(ctx, 0);
        std::string val = duk_to_string(ctx, 1);

        duk_eval_string(ctx, "ctx");
        KVSClient *m = (KVSClient *) duk_to_pointer(ctx, -1);
        // m->put(key, val, true, true, true);
        return 0;           
    }

    static duk_ret_t js_get(duk_context *ctx){
        std::string key = duk_to_string(ctx, 0);
    
        duk_eval_string(ctx, "ctx");
        KVSClient *m = (KVSClient *) duk_to_pointer(ctx, -1);

        duk_idx_t obj_idx = duk_push_object(ctx);
        // capsule_pdu dc = m->get(key);

        // duk_push_string(ctx, dc.payload.key.c_str());
        // duk_put_prop_string(ctx, obj_idx, "key");

        // duk_push_string(ctx, dc.payload.value.c_str());
        // duk_put_prop_string(ctx, obj_idx, "val");

        return 1;           
    }
} // namespace

} //namespace asylo 
