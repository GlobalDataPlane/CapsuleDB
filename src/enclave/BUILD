#
# Build file for the enclaved version of CapsuleDB
#

load("@linux_sgx//:sgx_sdk.bzl", "sgx")
load("@rules_cc//cc:defs.bzl", "cc_proto_library")
load("@rules_proto//proto:defs.bzl", "proto_library")
load("@com_google_asylo//asylo/bazel:asylo.bzl", "cc_unsigned_enclave", "debug_sign_enclave", "enclave_loader")
load("@com_google_asylo//asylo/bazel:copts.bzl", "ASYLO_DEFAULT_COPTS")

licenses(["notice"])

package(
    default_visibility = ["//visibility:public"],
)

# Protobuf Targets
proto_library(
    name = "request_proto",
    srcs = ["capsuleDBRequest.proto"],
    deps = [
        "//src/core:kvs_payload_proto",
        "@com_google_asylo//asylo:enclave_proto",
    ],
)

cc_proto_library(
    name = "request_cc_proto",
    deps = [":request_proto"]
)

proto_library(
    name = "setup_proto",
    srcs = ["capsuleDBSetup.proto"],
    deps = [
        "@com_google_asylo//asylo:enclave_proto",
    ],
)

cc_proto_library(
    name = "setup_cc_proto",
    deps = [":setup_proto"]
)

# Enclave Targets

cc_unsigned_enclave(
    name = "capsuleDB_enclave_unsigned.so",
    srcs = [
        "capsuleDB_enclave.cc",
    ],
    copts = ASYLO_DEFAULT_COPTS,
    deps = [
        "//src/core:capsuleEngine",
        "//src/shared:capsule",
        ":request_cc_proto",
        ":setup_cc_proto",
        "@com_google_asylo//asylo:enclave_runtime",
        "@com_google_asylo//asylo/util:status",
    ],
)

debug_sign_enclave(
    name = "capsuleDB_enclave.so",
    unsigned = "capsuleDB_enclave_unsigned.so",
)

enclave_loader(
    name = "capsuleDB_enclave",
    srcs = ["capsuleDB_driver.cc"],
    backends = sgx.backend_labels,  # Has SGX loader dependencies
    copts = ASYLO_DEFAULT_COPTS,
    enclaves = {"enclave": ":capsuleDB_enclave.so"},
    loader_args = ["--enclave_path='{enclave}'"],
    deps = [
        "//src/shared:capsule",
        ":demo_cc_proto",
        ":request_cc_proto",
        ":setup_cc_proto",
        "@com_google_absl//absl/flags:flag",
        "@com_google_absl//absl/flags:parse",
        "@com_google_asylo//asylo:enclave_cc_proto",
        "@com_google_asylo//asylo:enclave_client",
        "@com_google_asylo//asylo/util:logging",
    ] + select(
        {
            "@linux_sgx//:sgx_hw": ["@com_google_asylo//asylo/platform/primitives/sgx:loader_cc_proto"],
            "@linux_sgx//:sgx_sim": ["@com_google_asylo//asylo/platform/primitives/sgx:loader_cc_proto"],
        },
        no_match_error = "CapsuleDB is only configured to use the SGX backends",
    ),
)