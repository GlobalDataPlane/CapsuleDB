# Build file for shared resources.

package(
    default_visibility = ["//visibility:public"],
)

cc_library(
    name="benchmark",
    srcs=["benchmark.h"],
)

cc_library(
    name="capsule",
    srcs=["capsule.h"],
    deps=[":common"],
)

cc_library(
    name="common",
    srcs=["common.h"],
)

cc_library(
    name="crypto",
    srcs=["crypto.h"],
)
