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

# Protobuf targets
proto_library(
    name="kvs_payload_proto",
    srcs=["kvs_payload.proto"],
    deps=["@com_google_asylo//asylo:enclave_proto"],
)

cc_proto_library(
    name="kvs_payload_cc_proto",
    deps=[":kvs_payload_proto"],
)

proto_library(
    name="capsuleBlock_proto",
    srcs=[
        "capsuleBlock.proto",
        ":kvs_payload.proto"
    ],
    deps=["@com_google_asylo//asylo:enclave_proto"],
)

cc_proto_library(
    name="capsuleBlock_cc_proto",
    deps=[":capsuleBlock_proto"],
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
    ],
    deps=[
        "@boost//:archive",
        "@boost//:serialization",
        ":fakeCapsule",
        "//src/shared:capsule",
    ],
)

cc_library(
    name="fakeCapsule",
    srcs=[
        "fakeCapsule.cc",
    ],
    hdrs=[
        "fakeCapsule.hh",
        "capsuleBlock.hh",
    ],
    deps=[
        "//src/shared:common",
        "//src/shared:benchmark",
        "//src/shared:capsule",
        # "//src/shared:crypto",
        "//src/uuid_v4:uuid",
        "@boost//:archive",
        "@boost//:serialization",
        "@com_github_grpc_grpc//:grpc++",
        ":capsuleBlock_cc_proto",
        "@com_google_asylo//asylo/util:logging",
        "@com_google_absl//absl/strings",
    ],
)

cc_library(
    name="capsuleIndex",
    srcs=[
        "index.cc",
    ],
    hdrs=[
        "index.hh",
    ],
    deps=[
        ":capsuleBlock",
        ":capsuleLevel",
        "//src/bloom:bloom_filter",
    ],
)

cc_library(
    name="capsuleLevel",
    srcs=[
        "level.cc",
    ],
    hdrs=[
        "level.hh",
    ],
    deps=[
        ":fakeCapsule",
        ":capsuleBlock",
        "//src/bloom:bloom_filter",
    ],
)

cc_library(
    name="memtable",
    srcs=[
        "memtable.cc",
    ],
    hdrs=[
        "memtable.hpp",
    ],
    deps=[
        ":capsuleBlock",
        ":capsuleIndex",
        ":capsuleLevel",
        "//src/shared:capsule",
        "//src/shared:common",
        "//src/shared:benchmark",
        "@com_google_asylo//asylo/util:logging",
        "@com_google_absl//absl/container:flat_hash_map",
    ],
)

cc_library(
    name="capsuleEngine",
    srcs=[
        "engine.cc",
        ],
    hdrs=[
        "engine.hh", 
    ],
    deps=[
        ":memtable",
        ":capsuleIndex",
        "//src/shared:capsule",
        "//src/shared:common",
        "//src/shared:benchmark",
        "@com_google_absl//absl/strings",
    ],
)

