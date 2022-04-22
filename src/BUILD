# Testing and integration build file.  

# ONLY DO TESTING HERE, MAKE A SEPARATE BUILD FILE FOR PACKAGES


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

cc_binary(
    name="test_core",
    srcs=[
        "test_files/test_core.cc",
    ],
    deps=[
        "//src/core:capsuleEngine",
    ]
)

enclave_loader(
    name = "test_enclave_basic",
    srcs = ["test_files/test_enclave_basic.cc"],
    backends = sgx.backend_labels,  # Has SGX loader dependencies
    copts = ASYLO_DEFAULT_COPTS,
    enclaves = {"enclave": "//src/enclave:capsuleDB_enclave.so"},
    loader_args = ["--enclave_path='{enclave}'"],
    deps = [
        "//src/shared:capsule",
        "//src/enclave:request_cc_proto",
        "//src/enclave:setup_cc_proto",
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

enclave_loader(
    name = "test_enclave_interface",
    srcs = [
        "test_files/test_enclave_interface.cc",
    ],
    backends = sgx.backend_labels,  # Has SGX loader dependencies
    copts = ASYLO_DEFAULT_COPTS,
    enclaves = {"enclave": "//src/enclave:capsuleDB_enclave.so"},
    loader_args = ["--enclave_path='{enclave}'"],
    deps = [
        "//src/enclave:enclave_interface",
        "//src/shared:capsule",
        "//src/enclave:request_cc_proto",
        "//src/enclave:setup_cc_proto",
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