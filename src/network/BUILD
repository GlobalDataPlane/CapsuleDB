# Core CapsuleDB Implementation Build file

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
    name = "capsuleDB_server",
    srcs=[
        "capsuleDB_server.cc",
    ],
    linkopts=["-L/usr/local/lib", "-lzmq"],
    deps=[
        "//src/core:capsuleEngine",
        "//src/shared:capsule",
        "//src/enclave:request_cc_proto",
    ],
)

cc_binary(
    name = "networktest",
    srcs = [
        "networktest.cc",
    ],
    linkopts=["-L/usr/local/lib", "-lzmq"],
    deps = [
        "//src/shared:capsule",
        "//src/enclave:request_cc_proto",
    ]
)