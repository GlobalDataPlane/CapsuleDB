#
# Copyright 2018 Asylo authors
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

# Hello World project for Asylo.

load("@linux_sgx//:sgx_sdk.bzl", "sgx")
load("@com_github_grpc_grpc//bazel:cc_grpc_library.bzl", "cc_grpc_library")
load("@rules_cc//cc:defs.bzl", "cc_proto_library")
load("@rules_proto//proto:defs.bzl", "proto_library")
load("@com_google_asylo//asylo/bazel:asylo.bzl", "cc_enclave_binary", "cc_unsigned_enclave", "debug_sign_enclave", "enclave_loader", "ASYLO_ALL_BACKEND_TAGS")
load("@com_google_asylo//asylo/bazel:copts.bzl", "ASYLO_DEFAULT_COPTS")


licenses(["notice"])

package(
    default_visibility = ["//visibility:public"],
)

# Example for using the Asylo framework.

proto_library(
    name = "hello_proto",
    srcs = ["proto/hello.proto", "proto/capsule.proto"],
    deps = [
        "@com_google_asylo//asylo:enclave_proto",
        ":translator_server_proto",
    ],
)

cc_proto_library(
    name = "hello_cc_proto",
    deps = [":hello_proto"],
)

# CapsuleDB targets
cc_library(
    name = "capsuleBlock",
    srcs=[
        "core/capsuleBlock.cc",
],
    hdrs=[
        "core/capsuleBlock.hh",
        "core/fakeCapsule.hh",
        "kvs_include/capsule.h"
    ],
    deps=[
        "@boost//:archive",
        "@boost//:serialization",
        ":fakeCapsule",
    ],
)

cc_library(
    name="fakeCapsule",
    srcs=[
        "core/fakeCapsule.cc",
    ],
    hdrs=[
        "core/fakeCapsule.hh",
        "core/capsuleBlock.hh",
        "common.h",
        "benchmark.h",
        "kvs_include/capsule.h",
        "crypto.h"
    ],
    deps=[
        "@boost//:archive",
        "@boost//:serialization",
        "@com_github_grpc_grpc//:grpc++",
    ],
)

cc_library(
    name="capsuleIndex",
    srcs=[
        "core/index.cc",
    ],
    hdrs=[
        "core/index.hh",
        "bloom/bloom_filter.hpp",
    ],
    deps=[
        ":capsuleBlock",
        ":capsuleLevel",
    ],
)

cc_library(
    name="capsuleLevel",
    srcs=[
        "core/level.cc",
    ],
    hdrs=[
        "core/level.hh",
        "bloom/bloom_filter.hpp",
    ],
    deps=[
        ":fakeCapsule",
        ":capsuleBlock",
    ],
)

cc_library(
    name="memtable",
    srcs=[
        "core/memtable.cc",
    ],
    hdrs=[
        "core/memtable.hpp",
        "kvs_include/capsule.h",
        "common.h",
        "benchmark.h",
    ],
    deps=[
        ":capsuleBlock",
        ":capsuleIndex",
        ":capsuleLevel",
        "@com_google_asylo//asylo/util:logging",
        "@com_google_absl//absl/container:flat_hash_map",
    ],
)

cc_library(
    name="capsuleEngine",
    srcs=[
        "core/engine.cc",
        "benchmark.h",
        ],
    hdrs=[
        "core/engine.hh", 
        "kvs_include/capsule.h",
        "common.h",
        "benchmark.h",
    ],
    deps=[
        ":memtable",
        ":capsuleIndex",
        "@com_google_absl//absl/strings",
    ],
    visibility=[  # Change this entry for compability
        "//main:__pkg__"
    ]
)

cc_binary(
    name="capsuleDBTest",
    srcs=[
        "test_files/test.cc",
    ],
    deps=[
        ":capsuleEngine",
    ],
)

# CapsuleDB Integration Targets
# cc_library(
#     name="networkedCapsuleDB",
#     srcs=[
#         "network/cdb_network_client.cc"
#     ],
#     hdrs = [
#         "network/cdb_network_client.hh",
#         "core/engine.hh",
#         "kvs_include/capsule.h",
#     ],
#     deps=[
#         ":capsuleEngine",
#         ":hello_cc_proto",
#         ":proto_util",
#         "@com_google_asylo//asylo:enclave_client",
#         "@com_google_asylo//asylo/util:logging",
#         "@com_google_asylo//asylo/crypto:ecdsa_p256_sha256_signing_key",
#         "@com_google_asylo//asylo/crypto:aead_cryptor",
#         "@com_google_asylo//asylo/platform/primitives:trusted_primitives",
#         "@com_google_absl//absl/strings",
#         "@com_google_asylo//asylo/util:status",

#     ]
# )

# # # The implementation of the translation server.
# # cc_library(
# #     name = "translator_server_impl",
# #     srcs = ["translator_server_impl.cc"],
# #     hdrs = ["translator_server_impl.h"],
# #     copts = ASYLO_DEFAULT_COPTS,
# #     tags = ASYLO_ALL_BACKEND_TAGS,
# #     deps = [
# #         ":translator_server",
# #         "@com_google_asylo//asylo/grpc/auth:enclave_auth_context",
# #         "@com_google_asylo//asylo/identity:descriptions",
# #         "@com_google_asylo//asylo/identity:identity_acl_cc_proto",
# #         "@com_github_grpc_grpc//:grpc++",
# #         "@com_google_absl//absl/base:core_headers",
# #         "@com_google_absl//absl/container:flat_hash_map",
# #         # "@com_google_absl//absl/container:btree_set",
# #         "@com_google_absl//absl/strings",
# #         "@com_google_absl//absl/synchronization",
# #     ],
# # )

# # cc_library(
# #     name="grpc_server_util",
# #     srcs=["grpc_server_util.cc"],
# #     hdrs=["grpc_server_util.h"],
# #     copts=ASYLO_DEFAULT_COPTS,
# #     deps=[
# #         ":attestation_domain",
# #         ":grpc_server_config_cc_proto",
# #         "@com_google_asylo//asylo:enclave_cc_proto",
# #         "@com_google_asylo//asylo:enclave_client",
# #         "@com_google_asylo//asylo/examples/grpc_server:grpc_server_config_cc_proto",
# #         "@com_google_asylo//asylo/identity:enclave_assertion_authority_config_cc_proto",
# #         "@com_google_asylo//asylo/identity:enclave_assertion_authority_configs",
# #         "@com_google_asylo//asylo/identity/platform/sgx:sgx_identity_cc_proto",
# #         "@com_google_asylo//asylo/platform/primitives/sgx:loader_cc_proto",
# #         "@com_google_asylo//asylo/util:status",
# #         "@com_google_absl//absl/strings",
# #         "@com_google_protobuf//:protobuf",
# #     ],
# # )

# # cc_library(
# #     name="attestation_domain",
# #     srcs=["attestation_domain.cc"],
# #     hdrs=["attestation_domain.h"],
# # )

# # cc_proto_library(
# #     name="grpc_client_enclave_cc_proto",
# #     deps=[":grpc_client_enclave_proto"],
# # )

# # cc_proto_library(
# #     name="grpc_server_config_cc_proto",
# #     deps=[":grpc_server_config_proto"],
# # )

# # The generated code for the translator server (and client).
# proto_library(
#     name = "translator_server_proto",
#     srcs = ["translator_server.proto"],
#     tags = ASYLO_ALL_BACKEND_TAGS,
# )

# # cc_proto_library(
# #     name = "translator_server_cc_proto",
# #     tags = ASYLO_ALL_BACKEND_TAGS,
# #     deps = [":translator_server_proto"],
# # )

# # cc_grpc_library(
# #     name = "translator_server",
# #     srcs = [":translator_server_proto"],
# #     grpc_only = True,
# #     tags = ASYLO_ALL_BACKEND_TAGS,
# #     deps = [":translator_server_cc_proto"],

# # cc_binary(
# #     name="capsuleDBIntegTests",
# #     srcs=[
# #         "test_files/cdb_test.cc",
# #         "test_files/cdb_test.hh",
# #         "zmq_comm.hpp",
# #         "zmq_comm.cc",
# #         "kvs_include/capsule.h",
# #         "common.h",
# #         "benchmark.h",
# #         "crypto.h",
# #         "hot_msg_pass.h",
# #         "sgx_spinlock.h",
# #         "spinlock.c", 
# #         "asylo_sgx.hpp",
# #         "asylo_sgx.cc",
# #     ],
# #     linkopts=["-L/usr/local/lib", "-lzmq"],
# #     deps=[
# #         ":networkedCapsuleDB",
# #         ":capsuleDB_cc_proto",
# #         ":hello_cc_proto",
# #         ":proto_util",
# #         "@com_google_absl//absl/flags:flag",
# #         "@com_google_absl//absl/flags:parse",
# #         "@com_google_absl//absl/strings",
# #         "@com_google_asylo//asylo:enclave_cc_proto",
# #         "@com_google_asylo//asylo:enclave_client",
# #         "@com_google_asylo//asylo/platform/primitives/sgx:loader_cc_proto",
# #         "@com_google_asylo//asylo/util:logging",
# #         "@com_google_asylo//asylo/crypto:sha256_hash_util",
# #         "@com_google_asylo//asylo/identity:enclave_assertion_authority_configs",
# #         "@com_google_asylo//asylo/crypto:aead_cryptor",
# #         # "@com_google_asylo//asylo/crypto/util:byte_container_util"
# #     ],
# # )



# #CapsuleDB Enclave Targets

# cc_binary(
#     name="capsuleDB_driver",
#     srcs=[
#         "cdb_driver.cc",
#     ],
#     linkopts=["-L/usr/local/lib", "-lzmq"],
#     deps=[
#         "@com_google_absl//absl/flags:flag",
#         "@com_google_absl//absl/flags:parse",
#         "@com_google_absl//absl/strings",
#     ],
# )

# cc_library(
#     name="proto_util",
#     srcs=[
#         "util/proto_util.cc"
#     ],
#     hdrs=[
#         "util/proto_util.hpp",
#         "crypto.h",
#         "core/capsuleBlock.hh",
#         "core/index.hh",
#         "core/level.hh",
#         "kvs_include/capsule.h",
#         "common.h",
#         "benchmark.h",
#         # "proto/hello.pb.h"
#         "bloom/bloom_filter.hpp",
#     ],
#     deps=[
#         "@boost//:archive",
#         "@boost//:serialization",
#         "@com_google_asylo//asylo/crypto:ecdsa_p256_sha256_signing_key",
#         "@com_google_asylo//asylo/crypto:aead_cryptor",
#         ":hello_cc_proto",
#         "@com_google_asylo//asylo/util:logging",
#         "@com_google_absl//absl/time",
#     ]
# )


# proto_library(
#     name = "capsuleDB_proto",
#     srcs = [
#         "capsuleDBcpp/capsuleDB.proto"
#     ],
#     deps = [
#         "@com_google_asylo//asylo:enclave_proto"
#     ]
# )

# cc_proto_library(
#     name = "capsuleDB_cc_proto",
#     deps = [":capsuleDB_proto"]
# )

# # cc_unsigned_enclave(
# #     name="capsuleDB_enclave_unsigned.so",
# #     srcs=[
# #         "capsuleDBcpp/capsuleDB_enclave.cc"
# #     ],
# #     backends=sgx.backend_labels,
# #     deps=[
# #         ":capsuleEngine",
# #     ]
# # )


# cc_unsigned_enclave(
#     name = "capsuleDB_enclave_unsigned.so",
#     srcs = [
#         "capsuleDBcpp/capsuleDB_enclave.cc",
#         "capsuleDBcpp/capsuleDB_enclave.hh",
#         # "util/proto_util.hpp"
#     ],
#     copts = ASYLO_DEFAULT_COPTS,
#     deps=[
#         ":capsuleDB_cc_proto",
#         ":capsuleEngine",
#         # "@boost//:archive",
#         # "@boost//:serialization",
#         "@com_google_absl//absl/strings",
#         "@com_google_absl//absl/base:core_headers",
#         "@com_google_absl//absl/synchronization",
#         "@com_google_absl//absl/container:flat_hash_map",
#         "@com_google_asylo//asylo:enclave_runtime",
#         "@com_google_asylo//asylo/util:logging",
#         "@com_google_asylo//asylo/util:status",
#         "@com_google_asylo//asylo/crypto:aead_cryptor",
#         "@com_google_asylo//asylo/crypto:ecdsa_p256_sha256_signing_key",
#         "@com_google_asylo//asylo/crypto:sha256_hash_util",
#         "@com_google_asylo//asylo/platform/primitives:trusted_primitives",
#         "@com_google_asylo//asylo/identity/platform/sgx:sgx_identity_util",
#         "@com_google_asylo//asylo/identity/attestation/sgx:sgx_local_assertion_generator",
#         "@com_google_asylo//asylo/identity:enclave_assertion_authority_configs",
#         "@com_google_asylo//asylo/grpc/auth:grpc++_security_enclave",
#         "@com_google_asylo//asylo/grpc/auth:sgx_local_credentials_options",
#         "@com_google_absl//absl/time",
#     ],
# )

# debug_sign_enclave(
#     name = "capsuleDB_enclave.so",
#     unsigned = "capsuleDB_enclave_unsigned.so"
# )

# enclave_loader(
#     name = "capsuleDB",
#     srcs = [
#         "capsuleDBcpp/capsuleDB_driver.cc",
#         "kvs_include/capsule.h",
#         "common.h",
#         "benchmark.h",
#     ],
#     backends = sgx.backend_labels,  # Has SGX loader dependencies
#     copts = ASYLO_DEFAULT_COPTS,
#     enclaves = {"enclave": ":capsuleDB_enclave.so"},
#     loader_args = ["--enclave_path='{enclave}'"],
#     deps=[
#         ":capsuleDB_cc_proto",
#         "@com_google_absl//absl/flags:flag",
#         "@com_google_absl//absl/flags:parse",
#         "@com_google_absl//absl/strings",
#         "@com_google_asylo//asylo:enclave_cc_proto",
#         "@com_google_asylo//asylo:enclave_client",
#         "@com_google_asylo//asylo/util:logging",
#         "@com_google_asylo//asylo/crypto:sha256_hash_util",
#         "@com_google_asylo//asylo/identity:enclave_assertion_authority_configs",
#         "@com_google_asylo//asylo/crypto:aead_cryptor",
#     ] + select(
#         {
#             "@linux_sgx//:sgx_hw": ["@com_google_asylo//asylo/platform/primitives/sgx:loader_cc_proto"],
#             "@linux_sgx//:sgx_sim": ["@com_google_asylo//asylo/platform/primitives/sgx:loader_cc_proto"],
#         },
#         no_match_error="The grpc server enclave is only configured for SGX backends",
#     ),
# )
