# Testing and integration build file.  

# ONLY DO TESTING HERE, MAKE A SEPARATE BUILD FILE FOR PACKAGES

# Commands:

# MY_PROJECT=~/Documents/Research/paranoid-sgx

# docker run -it --rm \
#    -v bazel-cache:/root/.cache/bazel \
#     -v "${MY_PROJECT}":/opt/my-project \
#     -w /opt/my-project \
#     keplerc/paranoid-asylo:latest 

# bazel run //src:capsuleDBTest


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
    name="capsuleDBTest",
    srcs=[
        "test_files/test.cc",
    ],
    deps=[
        "//src/core:capsuleEngine",
    ],
)