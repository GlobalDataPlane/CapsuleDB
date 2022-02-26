#include "capsuleDB_enclave.hh"

#include <string>

#include "engine.hh"
#include "src/capsuleDBcpp/capsuleDB.pb.h"

namespace asylo {
    namespace {

    }  // namespace

    Status CapsuleDBClient::Initialize(const EnclaveConfig &config){
        LOG(INFO) << "Entered CapsuleDB enclave";
        // size_t blocksize = GetEnclaveBlocksize(config);
        size_t blocksize = 50
        db = CapsuleDB(blocksize);
        return asylo::Status::OkStatus();
    }

    Status CapsuleDBClient::Run(const EnclaveInput &input, EnclaveOutput *output) {
        const std::string requestedKey = GetEnclaveRequestedKey(input);

        if (requestedKey == "") {
            kvs_payload payload = GetEnclavePayload(input);
            db.put(payload);
        } else {
            kvs_payload payload = db.get(requestedKey);
            SetEnclaveOutputPayload(output, payload);
        }
       
        return absl::OkStatus();
    }

    int32_t CapsuleDBClient::GetEnclaveBlocksize(const EnclaveLoadConfig &config) {
        return config.GetExtension(capsuleDB::dbConfig).blocksize();
    }

    // Retrieves user message from |input|.
    const std::string CapsuleDBClient::GetEnclaveRequestedKey(const EnclaveInput &input) {
        return input.GetExtension(capsuleDB::capsuleDBEnclaveInput).requestedkey();
    }

    // Retrieves user action from |input|.
    kvs_payload CapsuleDBClient::GetEnclavePayload(const EnclaveInput &input) {
        const capsuleDB::DBRequest::kvs_payload& incoming_payload = input.GetExtension(capsuleDB::capsuleDBEnclaveOutput).payload();
        kvs_payload kvs;
        kvs.key = incoming_payload.key();
        kvs.value = incoming_payload.value();
        kvs.txn_timestamp = incoming_payload.timestamp();
        return kvs;
    }

    // Populates |enclave_output|->value() with |output_message|. Intended to be
    // used by the reader for completing the exercise.
    void CapsuleDBClient::SetEnclaveOutputPayload(EnclaveOutput *enclave_output,
                                kvs_payload retrieved_payload) {
        capsuleDB::DBRequest *output = enclave_output->MutableExtension(capsuleDB::capsuleDBEnclaveInput);
        capsuleDB::DBRequest::kvs_payload new_payload = output->payload();
        new_payload.set_key(retrieved_payload.key);
        new_payload.set_value(retrieved_payload.value);
        new_payload.set_timestamp(retrieved_payload.txn_timestamp);
    }

    TrustedApplication *BuildTrustedApplication() { return new CapsuleDBClient; }

}  // namespace asylo









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

// #include "capsuleDB_enclave.hpp"
// #include "../kvs_eapp.hpp"

// // #define USE_KEY_MANAGER

// namespace asylo {
//     void CapsuleDBClient::put(std::string key, std::string value, std::string msgType = "") {
//         kvs_payload payload;
//         asylo::KvToPayload(&payload, key, value, m_lamport_timer, msgType);
//         db.put(payload);
//     }

//     void CapsuleDBClient::get(const std::string &key){
//         kvs_payload result = db.get(key);
//         pqueue.enqueue(&payload);
//     }

//     void CapsuleDBClient::handle() {
//         // dequeue msg/txn from pqueue and then handle
//         std::vector<kvs_payload> payload_l = pqueue.dequeue(BATCH_SIZE);
//         if (payload_l.size() == 0){
//             return;
//         }
//         capsule_pdu *dc = new capsule_pdu();
//         asylo::PayloadListToCapsule(dc, &payload_l, m_enclave_id);

//         // generate hash for update_hash and/or ocall
//         bool success = encrypt_payload_l(dc);
//         if (!success) {
//             LOGI << "payload_l encryption failed!!!";
//             delete dc;
//             return;
//         }

//         // generate hash and update prev_hash
//         success = generate_hash(dc);
//         if (!success) {
//             LOGI << "hash generation failed!!!";
//             delete dc;
//             return;
//         }
//         dc->prevHash = m_prev_hash;
//         m_prev_hash = dc->hash;

//         // sign dc
//         success = sign_dc(dc, signing_key);
//         if (!success) {
//             LOGI << "sign dc failed!!!";
//             delete dc;
//             return;
//         }
//         DUMP_CAPSULE(dc);

//         // send dc
//         put_ocall(dc);
        
//         delete dc;
//     }

//     asylo::Status CapsuleDBClient::Initialize(const EnclaveConfig &config){
//         int32_t blocksize = config.GetExtension(capsuleDB::dbConfig).blocksize();
//         db = CapsuleDB(blocksize);
//         return asylo::Status::OkStatus();
//     }

//     // Fake client
//     asylo::Status CapsuleDBClient::Run(const asylo::EnclaveInput &input,
//                         asylo::EnclaveOutput *output) {

//         m_prev_hash = "init";
//         requestedCallID = 0;
//         m_lamport_timer = 0;

//         // Assign signing and verifying key
//         if (input.HasExtension(hello_world::crypto_param)) {
//             ASYLO_ASSIGN_OR_RETURN(signing_key,
//                     EcdsaP256Sha256SigningKey::CreateFromDer(input.GetExtension(hello_world::crypto_param).key()));
//             ASYLO_ASSIGN_OR_RETURN(verifying_key,
//                 signing_key->GetVerifyingKey());
//         }

//         if (input.HasExtension(hello_world::enclave_responder)) {
            
//         #ifdef USE_KEY_MANAGER
//             std::string server_addr = input.GetExtension(hello_world::kvs_server_config).server_address();
    
//             if (server_addr.empty()) {
//                 return absl::InvalidArgumentError(
//                     "Input must provide a non-empty server address");
//             }

//             int32_t port = input.GetExtension(hello_world::kvs_server_config).port();
//             server_addr = absl::StrCat(server_addr, ":", port);

//             LOG(INFO) << "Configured with KVS Address: " << server_addr;

//             // The ::grpc::ChannelCredentials object configures the channel authentication
//             // mechanisms used by the client and server. This particular configuration
//             // enforces that both the client and server authenticate using SGX local
//             // attestation.
//             std::shared_ptr<::grpc::ChannelCredentials> channel_credentials =
//                 EnclaveChannelCredentials(
//                     asylo::BidirectionalSgxLocalCredentialsOptions());

//             // Connect a gRPC channel to the server specified in the EnclaveInput.
//             std::shared_ptr<::grpc::Channel> channel =
//                 ::grpc::CreateChannel(server_addr, channel_credentials);

//             gpr_timespec absolute_deadline = gpr_time_add(
//                 gpr_now(GPR_CLOCK_REALTIME),
//                 gpr_time_from_micros(absl::ToInt64Microseconds(kChannelDeadline),
//                                     GPR_TIMESPAN));
//             if (!channel->WaitForConnected(absolute_deadline)) {
//                 LOG(INFO) << "Failed to connect to server";  

//                 //return absl::InternalError("Failed to connect to server");
//             } else {
//                 LOG(INFO) << "Successfully connected to server";

//                 hello_world::GrpcClientEnclaveInput client_input;
//                 hello_world::GrpcClientEnclaveOutput client_output;

//                 std::unique_ptr <Translator::Stub> stub = Translator::NewStub(channel);

//                 ASYLO_ASSIGN_OR_RETURN(
//                         *client_output.mutable_key_pair_response(),
//                         RetrieveKeyPair(client_input.key_pair_request(), stub.get()));

//                 RetrieveKeyPairResponse resp = *client_output.mutable_key_pair_response();

//                 priv_key = resp.private_key();
//                 pub_key = resp.public_key();

//                 LOG(INFO) << "CapsuleDB enclave configured with private key: " << priv_key << " public key: "
//                             << pub_key;
//             }
//         #endif

//             HotMsg *hotmsg = (HotMsg *) input.GetExtension(hello_world::enclave_responder).responder();
//             EnclaveMsgStartResponder(hotmsg);
//             return asylo::Status::OkStatus();
//         } else if (input.HasExtension(hello_world::is_actor_thread)){
//             while(true){
//                 handle();
//             }
//             return asylo::Status::OkStatus();
//         } else {
//             is_coordinator = false;
//         }

//         //Register OCALL buffer
//         buffer = (HotMsg *) input.GetExtension(hello_world::buffer).buffer();
//         m_enclave_id = std::stoi(input.GetExtension(hello_world::buffer).enclave_id());

//         sleep(3);

//         if (input.HasExtension(hello_world::lambda_input)){
//             return start_eapp(this, input);
//         } 
//         // TODO: there still has some issues when the client starts before the client connects to the server
//         // if we want to consider it, probably we need to buffer the messages
//         return asylo::Status::OkStatus();
//     }

//     /*
//         We can allocate OCALL params on stack because params are copied to circular buffer.
//     */
//     void CapsuleDBClient::put_ocall(capsule_pdu *dc){
//         OcallParams args;
//         args.ocall_id = OCALL_PUT;
//         args.data = dc;
//         HotMsg_requestOCall( buffer, requestedCallID++, &args);
//     }

//     int CapsuleDBClient::HotMsg_requestOCall( HotMsg* hotMsg, int dataID, void *data ) {
//         int i = 0;
//         const uint32_t MAX_RETRIES = 10;
//         uint32_t numRetries = 0;
//         int data_index = dataID % (MAX_QUEUE_LENGTH - 1);

//         //Request call
//         while( true ) {
//             HotData* data_ptr = (HotData*) hotMsg -> MsgQueue[data_index];
//             sgx_spin_lock( &data_ptr->spinlock );

//             if( data_ptr-> isRead == true ) {
//                 data_ptr-> isRead  = false;
//                 OcallParams *arg = (OcallParams *) data;
//                 hello_world::CapsulePDU out_dc;
//                 asylo::CapsuleToProto((capsule_pdu *) arg->data, &out_dc);

//                 std::string out_s;
//                 out_dc.SerializeToString(&out_s);
//                 data_ptr->data = primitives::TrustedPrimitives::UntrustedLocalAlloc(out_s.size());
//                 data_ptr->size = out_s.size();    
//                 memcpy(data_ptr->data, out_s.c_str(), data_ptr->size);

//                 data_ptr->ocall_id = arg->ocall_id;
//                 sgx_spin_unlock( &data_ptr->spinlock );
//                 break;
//             }
//             //else:
//             sgx_spin_unlock( &data_ptr->spinlock );

//             numRetries++;
//             if( numRetries > MAX_RETRIES ){
//                 LOGI << "exceeded tries\n";
//                 sgx_spin_unlock( &data_ptr->spinlock );
//                 return -1;
//             }

//             for( i = 0; i<3; ++i)
//                 _mm_sleep();
//         }

//         return numRetries;
//     }

//     void CapsuleDBClient::EnclaveMsgStartResponder( HotMsg *hotMsg )
//     {
//         int dataID = 0;

//         static int i;
//         sgx_spin_lock(&hotMsg->spinlock );
//         hotMsg->initialized = true;
//         sgx_spin_unlock(&hotMsg->spinlock);

//         while( true )
//         {

//             if( hotMsg->keepPolling != true ) {
//                 break;
//             }

//             HotData* data_ptr = (HotData*) hotMsg -> MsgQueue[dataID];

//             sgx_spin_lock( &data_ptr->spinlock );
//             if (data_ptr == 0){
//                 sgx_spin_unlock( &data_ptr->spinlock );
//                 continue;
//             }

//             if(data_ptr->data){
//                 //Message exists!
//                 EcallParams *arg = (EcallParams *) data_ptr->data;

//                 char *code = (char *) arg->data;
//                 capsule_pdu *dc = new capsule_pdu(); // freed below
//                 CapsuleToCapsule(dc, (capsule_pdu *) arg->data);
//                 primitives::TrustedPrimitives::UntrustedLocalFree((capsule_pdu *) arg->data);
//                 m_lamport_timer = std::max(m_lamport_timer, dc->timestamp) + 1;
//                 switch(arg->ecall_id){
//                     case ECALL_PUT:
//                         LOGI << "[CICBUF-ECALL] transmitted a data capsule pdu";
//                         if (verify_dc(dc, verifying_key)) {
//                             LOGI << "dc verification successful.";
//                         } else {
//                             LOGI << "dc verification failed!!!";
//                         }
//                         // decrypt payload_l
//                         if (decrypt_payload_l(dc)) {
//                             LOGI << "dc payload_l decryption successful";
//                         } else {
//                             LOGI << "dc payload_l decryption failed!!!";
//                             break;
//                         }
//                         DUMP_CAPSULE(dc);
//                         // once received RTS, send the latest EOE
//                         if (dc->msgType == COORDINATOR_RTS_TYPE && !is_coordinator) {
//                             put(COORDINATOR_EOE_TYPE, m_prev_hash, COORDINATOR_EOE_TYPE);
//                             break;
//                         }
//                         else if (dc->msgType == COORDINATOR_EOE_TYPE && is_coordinator){
//                             // store EOE for future sync
//                             std::pair<std::string, int64_t> p;
//                             p.first = dc->payload_l[0].value;
//                             p.second = dc->timestamp;
//                             m_eoe_hashes[dc->sender] = p;
//                             // if EOE from all enclaves received, start sync 
//                             if(m_eoe_hashes.size() == TOTAL_THREADS - 2) { //minus 2 for server thread and coordinator thread
//                                 LOGI << "coordinator received all EOEs, sending report" << serialize_eoe_hashes();
//                                 put(COORDINATOR_SYNC_TYPE, serialize_eoe_hashes(), COORDINATOR_SYNC_TYPE);
//                                 // clear this epoch's EOE
//                                 m_eoe_hashes.clear();
//                             }
//                         }
//                         else if (dc->msgType == COORDINATOR_SYNC_TYPE && !is_coordinator ){
//                             compare_eoe_hashes_from_string(dc->payload_l[0].value);
//                             LOGI << "Received the sync report " << serialize_eoe_hashes();
//                             m_prev_hash = dc -> hash;
//                             // the following writes hash points to the prev sync point
//                             std::pair<std::string, int64_t> p;
//                             p.first = dc->hash;
//                             p.second = dc->timestamp;
//                             m_eoe_hashes[m_enclave_id] = p;
//                         }
//                         else {
//                             update_client_hash(dc);
//                             for (int i = 0; i < dc->payload_l.size(); i++) {
//                                 memtable.put(&(dc->payload_l[i]));
//                             }
//                         }
//                         break;
//                     case ECALL_RUN:
//                         duk_eval_string(ctx, code);
//                         break;
//                     default:
//                         LOGI << "Invalid ECALL id: %d\n", arg->ecall_id;
//                 }
//                 delete dc;
//                 primitives::TrustedPrimitives::UntrustedLocalFree(arg);
//                 data_ptr->data = 0;
//             }

//             data_ptr->isRead      = true;
//             sgx_spin_unlock( &data_ptr->spinlock );
//             dataID = (dataID + 1) % (MAX_QUEUE_LENGTH - 1);
//             for( i = 0; i<3; ++i)
//                 _mm_pause();
//         }
//     }

//     M_BENCHMARK_CODE

//     TrustedApplication *BuildTrustedApplication() { return new KVSClient; }

// }  // namespace asylo


