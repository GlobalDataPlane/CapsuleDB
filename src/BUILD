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

cc_library(
    name = "mpl_data",
    data = [
        "src/mplambda/resources/se3/Twistycool_env.dae",
        "src/mplambda/resources/se3/Twistycool_robot.dae"
    ],
)

cc_library(
    name = "mpl_hdrs",
    hdrs = glob(["mplambda/include/**"]),
    includes = ["mplambda/include", "kvs_include"],
)

cc_binary(
    name = "mp_coordinator.so",
    srcs = [
        "mplambda/src/mpl_coordinator.cpp",
        "mplambda/src/mpl/write_queue.cpp",
    ],
    copts = ["-std=c++17"],
    deps = [
        ":mpl_hdrs",
        "@eigen//:eigen3",
    ]
)

debug_sign_enclave(
    name = "mp_lambda_enclave.so",
    config = ":grpc_client_config",
    unsigned = "mp_lambda.so",
)

cc_unsigned_enclave(
    name = "mp_lambda.so",
    srcs = [
        "mplambda/src/mpl_lambda_pseudo.cpp",
        "mplambda/src/mpl/comm.cpp",
        "mplambda/src/mpl/write_queue.cpp",
        "mplambda/src/mpl/demo/lambda_common.cpp",
        "mplambda/src/mpl/demo/app_options.cpp",
        ],
    deps = [
        ":hello_cc_proto",
        "@com_google_asylo//asylo:enclave_runtime",
        ":mpl_hdrs",
        "@eigen//:eigen3",
        "@nigh//:nigh",
        "@libccd//:libccd",
        "@fcl//:fcl",
        "@assimp//:assimp",
    ],
    copts = ASYLO_DEFAULT_COPTS,
)

enclave_loader(
    name = "lambda_driver",
    srcs = ["lambda_driver.cc"],
    copts = ASYLO_DEFAULT_COPTS,
    enclaves = {"enclave": ":mp_lambda_enclave.so"},
    loader_args = [
        "--enclave_path='{enclave}'",
        "--coordinator=localhost",
        "--algorithm=rrt", 
        "--scenario se3",
        "--start=0,0,0,1,270,160,-200",
        "--goal=0,0,0,1,270,160,-400",
        "--min=53.46,-21.25,-476.86",
        "--max=402.96,269.25,-91.0",
        "--robot Twistycool_robot.dae",
        "--time_limit 60",
        "--env Twistycool_env.dae",
        ],
    deps = [
        ":hello_cc_proto",
        "@com_google_asylo//asylo:enclave_client",
        "@com_google_asylo//asylo/identity/platform/sgx:sgx_identity_cc_proto",
        "@com_google_asylo//asylo/util:logging",
        "@com_google_asylo//asylo/util:proto_flag",
        "@com_google_asylo//asylo/util:status",
        "@com_google_absl//absl/flags:flag",
        "@com_google_absl//absl/flags:parse",
        "@com_google_absl//absl/time",
        "@com_google_protobuf//:protobuf",
    ],
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

# The generated code for the translator server (and client).
proto_library(
    name = "translator_server_proto",
    srcs = ["translator_server.proto"],
    tags = ASYLO_ALL_BACKEND_TAGS,
)

cc_proto_library(
    name = "translator_server_cc_proto",
    tags = ASYLO_ALL_BACKEND_TAGS,
    deps = [":translator_server_proto"],
)

cc_grpc_library(
    name = "translator_server",
    srcs = [":translator_server_proto"],
    grpc_only = True,
    tags = ASYLO_ALL_BACKEND_TAGS,
    deps = [":translator_server_cc_proto"],
)


# The implementation of the translation server.
cc_library(
    name = "translator_server_impl",
    srcs = ["translator_server_impl.cc"],
    hdrs = ["translator_server_impl.h"],
    copts = ASYLO_DEFAULT_COPTS,
    tags = ASYLO_ALL_BACKEND_TAGS,
    deps = [
        ":translator_server",
        "@com_google_asylo//asylo/grpc/auth:enclave_auth_context",
        "@com_google_asylo//asylo/identity:descriptions",
        "@com_google_asylo//asylo/identity:identity_acl_cc_proto",
        "@com_github_grpc_grpc//:grpc++",
        "@com_google_absl//absl/base:core_headers",
        "@com_google_absl//absl/container:flat_hash_map",
        # "@com_google_absl//absl/container:btree_set",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/synchronization",
    ],
)

cc_library(
    name="grpc_server_util",
    srcs=["grpc_server_util.cc"],
    hdrs=["grpc_server_util.h"],
    copts=ASYLO_DEFAULT_COPTS,
    deps=[
        ":attestation_domain",
        ":grpc_server_config_cc_proto",
        "@com_google_asylo//asylo:enclave_cc_proto",
        "@com_google_asylo//asylo:enclave_client",
        "@com_google_asylo//asylo/examples/grpc_server:grpc_server_config_cc_proto",
        "@com_google_asylo//asylo/identity:enclave_assertion_authority_config_cc_proto",
        "@com_google_asylo//asylo/identity:enclave_assertion_authority_configs",
        "@com_google_asylo//asylo/identity/platform/sgx:sgx_identity_cc_proto",
        "@com_google_asylo//asylo/platform/primitives/sgx:loader_cc_proto",
        "@com_google_asylo//asylo/util:status",
        "@com_google_absl//absl/strings",
        "@com_google_protobuf//:protobuf",
    ],
)

cc_library(
    name="attestation_domain",
    srcs=["attestation_domain.cc"],
    hdrs=["attestation_domain.h"],
)

cc_proto_library(
    name="grpc_client_enclave_cc_proto",
    deps=[":grpc_client_enclave_proto"],
)

cc_proto_library(
    name="grpc_server_config_cc_proto",
    deps=[":grpc_server_config_proto"],
)

# Contains extensions to enclave protos.
proto_library(
    name="grpc_server_config_proto",
    srcs=["grpc_server_config.proto"],
    deps=[
        "@com_google_asylo//asylo:enclave_proto",
        "@com_google_asylo//asylo/identity/platform/sgx:sgx_identity_proto",
    ],
)


_grpc_server_sgx_deps = [
    "@com_google_asylo//asylo/identity/platform/sgx:sgx_identity_cc_proto",
    "@com_google_asylo//asylo/identity/platform/sgx:sgx_identity_expectation_matcher",
    "@com_google_asylo//asylo/identity/platform/sgx:sgx_identity_util",
]

cc_library(
    name="duktape",
    srcs=["duktape/duktape.c"],
    hdrs=["duktape/duktape.h",  "duktape/duk_config.h"],
    textual_hdrs=["capsuleDBcpp/index.cc"]
)

# CapsuleDB targets
cc_library(
    name = "capsuleBlock",
    srcs=[
        "capsuleBlock.cc",
],
    hdrs=[
        "capsuleBlock.hh",
        "fakeCapsule.hh",
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
        "fakeCapsule.cc",
    ],
    hdrs=[
        "fakeCapsule.hh",
        "common.h",
        "kvs_include/capsule.h",
        "capsuleBlock.hh",
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
        "index.cc",
    ],
    hdrs=[
        "index.hh",
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
        "level.cc",
    ],
    hdrs=[
        "level.hh",
        "bloom/bloom_filter.hpp",
    ],
    deps=[
        ":fakeCapsule",
        ":capsuleBlock",
    ],
)

cc_library(
    name="memtable_new",
    srcs=[
        "memtable_new.cc",
    ],
    hdrs=[
        "memtable_new.hpp",
        "kvs_include/capsule.h",
        "common.h",
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
        "engine.cc",
        "benchmark.h",
        ],
    hdrs=[
        "engine.hh", 
        "kvs_include/capsule.h",
        "common.h"
    ],
    deps=[
        ":memtable_new",
        ":capsuleIndex",
        "@com_google_absl//absl/strings",
    ],
)

cc_binary(
    name="capsuleDBTest",
    srcs=[
        "Test Files/test.cc",
    ],
    deps=[
        ":capsuleEngine",
    ],
)

# cc_library(
#     name="proto_util",
#     srcs=[
#         "util/proto_util.cc"
#     ],
#     hdrs=[
#         "util/proto_util.hpp",
#         "crypto.h",
#         "capsuleDBcpp/capsuleBlock.hh",
#         "capsuleDBcpp/index.hh",
#         "capsuleDBcpp/level.hh",
#         "kvs_include/capsule.h",
#         "common.h",
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

proto_library(
    name = "capsuleDB_proto",
    srcs = [
        "capsuleDBcpp/capsuleDB.proto"
    ],
    deps = [
        "@com_google_asylo//asylo:enclave_proto"
    ]
)

cc_proto_library(
    name = "capsuleDB_cc_proto",
    deps = [":capsuleDB_proto"]
)

# cc_unsigned_enclave(
#     name="capsuleDB_enclave_unsigned.so",
#     srcs=[
#         "capsuleDBcpp/capsuleDB_enclave.cc"
#     ],
#     backends=sgx.backend_labels,
#     deps=[
#         ":capsuleEngine",
#     ]
# )

cc_unsigned_enclave(
    name = "capsuleDB_enclave_unsigned.so",
    srcs = [
        "capsuleDBcpp/capsuleDB_enclave.cc",
        "capsuleDBcpp/capsuleDB_enclave.hh"
    ],
    copts = ASYLO_DEFAULT_COPTS,
    deps=[
        ":hello_cc_proto",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/base:core_headers",
        "@com_google_absl//absl/synchronization",
        "@com_google_absl//absl/container:flat_hash_map",
        "@com_google_asylo//asylo:enclave_runtime",
        "@com_google_asylo//asylo/util:logging",
        "@com_google_asylo//asylo/util:status",
        "@com_google_asylo//asylo/crypto:aead_cryptor",
        "@com_google_asylo//asylo/crypto:ecdsa_p256_sha256_signing_key",
        "@com_google_asylo//asylo/crypto:sha256_hash_util",
        "@com_google_asylo//asylo/platform/primitives:trusted_primitives",
        "@com_google_asylo//asylo/identity/platform/sgx:sgx_identity_util",
        "@com_google_asylo//asylo/identity/attestation/sgx:sgx_local_assertion_generator",
        "@com_google_asylo//asylo/identity:enclave_assertion_authority_configs",
        "@com_google_asylo//asylo/grpc/auth:grpc++_security_enclave",
        "@com_google_asylo//asylo/grpc/auth:sgx_local_credentials_options",
        "@com_google_absl//absl/time",
    ],
)

debug_sign_enclave(
    name = "capsuleDB_enclave.so",
    unsigned = "capsuleDB_enclave_unsigned.so"
)

enclave_loader(
    name = "capsuleDB",
    srcs = [
        "capsuleDBcpp/capsuleDB_driver.cc"
    ],
    backends = sgx.backend_labels,  # Has SGX loader dependencies
    copts = ASYLO_DEFAULT_COPTS,
    enclaves = {"enclave": ":capsuleDB_enclave.so"},
    loader_args = ["--enclave_path='{enclave}'"],
    deps=[
        ":hello_cc_proto",
        "@com_google_absl//absl/flags:flag",
        "@com_google_absl//absl/flags:parse",
        "@com_google_absl//absl/strings",
        "@com_google_asylo//asylo:enclave_cc_proto",
        "@com_google_asylo//asylo:enclave_client",
        "@com_google_asylo//asylo/platform/primitives/sgx:loader_cc_proto",
        "@com_google_asylo//asylo/util:logging",
        "@com_google_asylo//asylo/crypto:sha256_hash_util",
        "@com_google_asylo//asylo/identity:enclave_assertion_authority_configs",
        "@com_google_asylo//asylo/crypto:aead_cryptor",
    ] + select(
        {
            "@linux_sgx//:sgx_hw": ["//asylo/platform/primitives/sgx:loader_cc_proto"],
            "@linux_sgx//:sgx_sim": ["//asylo/platform/primitives/sgx:loader_cc_proto"],
        },
        no_match_error="The grpc server enclave is only configured for SGX backends",
    ),
)

# End CapsuleDB build targets

cc_unsigned_enclave(
    name="hello_enclave_unsigned.so",
    srcs=[
        "kvs_enclave.cc",
        "kvs_include/kvs_enclave.hpp",
        "kvs_eapp.cc",
        "kvs_eapp.hpp",
        "memtable.hpp",
        "memtable.cc",
        "kvs_include/capsule.h",
        "util/proto_util.hpp",
        "util/proto_util.cc",
        "hot_msg_pass.h",
        "common.h",
        "benchmark.h",
        "crypto.h",
        "pqueue.hpp",
        "pqueue.cc"] + [
        #"mplambda/src/mpl_lambda_pseudo.cpp",
        "mplambda/src/mpl/comm.cpp",
        "mplambda/src/mpl/write_queue.cpp",
        "mplambda/src/mpl/demo/lambda_common.cpp",
        "mplambda/src/mpl/demo/app_options.cpp",
    ],
    backends=sgx.backend_labels,  # Has SGX identity dependencies
    deps=[
        ":hello_cc_proto",
        ":mpl_hdrs",
        ":mpl_data",
        "@eigen//:eigen3",
        "@nigh//:nigh",
        "@libccd//:libccd",
        "@fcl//:fcl",
        "@assimp//:assimp",
        ":duktape",
        ":translator_server",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/base:core_headers",
        "@com_google_absl//absl/synchronization",
        "@com_google_absl//absl/container:flat_hash_map",
        "@com_google_asylo//asylo:enclave_runtime",
        "@com_google_asylo//asylo/util:logging",
        "@com_google_asylo//asylo/util:status",
        "@com_google_asylo//asylo/crypto:aead_cryptor",
        "@com_google_asylo//asylo/crypto:ecdsa_p256_sha256_signing_key",
        "@com_google_asylo//asylo/crypto:sha256_hash_util",
        "@com_google_asylo//asylo/platform/primitives:trusted_primitives",
        "@com_google_asylo//asylo/identity/platform/sgx:sgx_identity_util",
        "@com_google_asylo//asylo/identity/attestation/sgx:sgx_local_assertion_generator",
        "@com_google_asylo//asylo/identity:enclave_assertion_authority_configs",
        "@com_google_asylo//asylo/grpc/auth:grpc++_security_enclave",
        "@com_google_asylo//asylo/grpc/auth:sgx_local_credentials_options",
        "@com_github_grpc_grpc//:grpc++",
        "@com_google_absl//absl/time",
    ],
)

debug_sign_enclave(
    name="hello_enclave.so",
    config=":grpc_client_config",
    unsigned="hello_enclave_unsigned.so",
)

# The enclave hosting the translation server.
cc_unsigned_enclave(
    name="grpc_server_enclave_unsigned.so",
    srcs=["grpc_server_enclave.cc"],
    backends=sgx.backend_labels,  # Has SGX identity dependencies
    copts=ASYLO_DEFAULT_COPTS,
    deps=[
        ":grpc_server_config_cc_proto",
        ":translator_server_impl",
        "@com_google_absl//absl/base:core_headers",
        "@com_google_absl//absl/memory",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/synchronization",
        "@com_google_asylo//asylo:enclave_runtime",
        "@com_google_asylo//asylo/examples/grpc_server:grpc_server_config_cc_proto",
        "@com_google_asylo//asylo/grpc/auth:grpc++_security_enclave",
        "@com_google_asylo//asylo/grpc/auth:sgx_local_credentials_options",
        "@com_google_asylo//asylo/identity:identity_acl_cc_proto",
        "@com_google_asylo//asylo/util:status",
        "@com_github_grpc_grpc//:grpc++",
        "@com_github_grpc_grpc//:grpc++_reflection",
    ] + select(
        {
            "@linux_sgx//:sgx_hw": _grpc_server_sgx_deps,
            "@linux_sgx//:sgx_sim": _grpc_server_sgx_deps,
        },
        no_match_error="The grpc server enclave is only configured for SGX backends",
    ),
)


sgx.enclave_configuration(
    name="grpc_client_config",
    isvsvn="3",
    prodid="2",
)


debug_sign_enclave(
    name="grpc_server_enclave.so",
    backends=sgx.backend_labels,
    config="@com_google_asylo//asylo/grpc/util:grpc_enclave_config",
    unsigned=":grpc_server_enclave_unsigned.so",
)

# The driver for the gRPC server enclave.
enclave_loader(
    name="grpc_server",
    srcs=["grpc_server_main.cc"],
    copts=ASYLO_DEFAULT_COPTS,
    enclaves={"enclave": ":grpc_server_enclave.so"},
    loader_args=[
        "--enclave_path='{enclave}'"],
    deps=[
        ":grpc_server_util",
        "@com_google_asylo//asylo:enclave_client",
        "@com_google_asylo//asylo/identity/platform/sgx:sgx_identity_cc_proto",
        "@com_google_asylo//asylo/util:logging",
        "@com_google_asylo//asylo/util:proto_flag",
        "@com_google_asylo//asylo/util:status",
        "@com_google_absl//absl/flags:flag",
        "@com_google_absl//absl/flags:parse",
        "@com_google_absl//absl/time",
        "@com_google_protobuf//:protobuf",
    ],
)

enclave_loader(
    name="hello_world",
    srcs=["pqueue.hpp", "zmq_comm.hpp", "zmq_comm.cc", "asylo_sgx.hpp", "asylo_sgx.cc", "sgx_spinlock.h", "spinlock.c", "hello_driver.cc", "kvs_include/capsule.h", "util/proto_util.hpp", "util/proto_util.cc", "hot_msg_pass.h",  "common.h", "benchmark.h", "crypto.h"],
    backends=sgx.backend_labels,  # Has SGX loader dependencies
    enclaves={"enclave": ":hello_enclave.so"},
    loader_args=[
        "--enclave_path='{enclave}'",
        "--mode=3",
        "--hosts=localhost,35.247.29.237",

        # Fetch params
        "--scenario fetch",
        "--algorithm cforest",
        "--coordinator=localhost",
        "--env AUTOLAB.dae",
        "--env_frame=0.57,-0.90,0.00,0,0,-1.570796326794897",
        "--goal=-1.07,0.16,0.88,0,0,0",
        "--goal_radius=0.01,0.01,0.01,0.01,0.01,3.141592653589793",
        "--start=0.1,1.570796326794897,1.570796326794897,0,1.570796326794897,0,1.570796326794897,0",
        "--time_limit 300",
        "--check_resolution 0.01",
        "--jobs 1",
    ],
    linkopts=["-L/usr/local/lib", "-lzmq"],
    deps=[
        ":mpl_data",
        ":hello_cc_proto",
        "@com_google_absl//absl/flags:flag",
        "@com_google_absl//absl/flags:parse",
        "@com_google_absl//absl/strings",
        "@com_google_asylo//asylo:enclave_cc_proto",
        "@com_google_asylo//asylo:enclave_client",
        "@com_google_asylo//asylo/platform/primitives/sgx:loader_cc_proto",
        "@com_google_asylo//asylo/util:logging",
        "@com_google_asylo//asylo/crypto:sha256_hash_util",
        "@com_google_asylo//asylo/identity:enclave_assertion_authority_configs",
        "@com_google_asylo//asylo/crypto:aead_cryptor",
    ],
)

enclave_loader(
    name="listener",
    srcs=["zmq_comm.hpp", "zmq_comm.cc", "asylo_sgx.hpp", "asylo_sgx.cc", "sgx_spinlock.h", "spinlock.c", "hello_driver.cc", "kvs_include/capsule.h", "util/proto_util.hpp", "util/proto_util.cc", "hot_msg_pass.h",  "common.h", "benchmark.h", "crypto.h"],
    backends=sgx.backend_labels,  # Has SGX loader dependencies
    enclaves={"enclave": ":hello_enclave.so"},
    loader_args=[
        "--enclave_path='{enclave}'",
        "--mode=2",
    ],
    linkopts=["-L/usr/local/lib", "-lzmq"],
    deps=[
        ":mpl_data",
        ":hello_cc_proto",
        "@com_google_absl//absl/flags:flag",
        "@com_google_absl//absl/flags:parse",
        "@com_google_absl//absl/strings",
        "@com_google_asylo//asylo:enclave_cc_proto",
        "@com_google_asylo//asylo:enclave_client",
        "@com_google_asylo//asylo/platform/primitives/sgx:loader_cc_proto",
        "@com_google_asylo//asylo/util:logging",
        "@com_google_asylo//asylo/crypto:sha256_hash_util",
        "@com_google_asylo//asylo/identity:enclave_assertion_authority_configs",
        "@com_google_asylo//asylo/crypto:aead_cryptor",
    ],
)

enclave_loader(
    name="js",
    srcs=["zmq_comm.hpp", "zmq_comm.cc", "asylo_sgx.hpp", "asylo_sgx.cc", "sgx_spinlock.h", "spinlock.c", "hello_driver.cc",
          "kvs_include/capsule.h", "util/proto_util.hpp", "util/proto_util.cc", "hot_msg_pass.h",  "common.h", "benchmark.h", "crypto.h"],
    backends=sgx.backend_labels,  # Has SGX loader dependencies
    enclaves={"enclave": ":hello_enclave.so"},
    loader_args=[
        "--enclave_path='{enclave}'",
        "--mode=4",
    ],
    linkopts=["-L/usr/local/lib", "-lzmq"],
    deps=[
        ":mpl_data",
        ":hello_cc_proto",
        "@com_google_absl//absl/flags:flag",
        "@com_google_absl//absl/flags:parse",
        "@com_google_absl//absl/strings",
        "@com_google_asylo//asylo:enclave_cc_proto",
        "@com_google_asylo//asylo:enclave_client",
        "@com_google_asylo//asylo/platform/primitives/sgx:loader_cc_proto",
        "@com_google_asylo//asylo/util:logging",
        "@com_google_asylo//asylo/crypto:sha256_hash_util",
        "@com_google_asylo//asylo/crypto:aead_cryptor",
        "@com_google_asylo//asylo/identity:enclave_assertion_authority_configs",
        "@com_google_asylo//asylo/crypto/util:byte_container_util"
    ],
)
