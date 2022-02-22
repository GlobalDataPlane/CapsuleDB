# Core CapsuleDB Implementation Build file

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