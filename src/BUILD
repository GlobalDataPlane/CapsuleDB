#
# Build file for CapsuleDB
# 

licenses(["notice"])

package(
    default_visibility = ["//visibility:public"],
)

# Standalone CapsuleDB targets
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
