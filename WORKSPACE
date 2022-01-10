workspace(name = "capsuleDB_engine1")

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")
load("@bazel_tools//tools/build_defs/repo:git.bzl", "git_repository", "new_git_repository")

git_repository(
    name = "com_github_nelhage_rules_boost",
    commit = "fce83babe3f6287bccb45d2df013a309fa3194b8",
    remote = "https://github.com/nelhage/rules_boost",
    shallow_since = "1591047380 -0700",
)
load("@com_github_nelhage_rules_boost//:boost/boost.bzl", "boost_deps")
boost_deps()

# Download and use the Asylo SDK.

# http_archive(
#     name = "com_google_asylo",
#     sha256 = "bb6e9599f3e174321d96616ac8069fac76ce9d2de3bd0e4e31e1720c562e83f7",
#     strip_prefix = "asylo-0.6.0",
#     urls = ["https://github.com/google/asylo/archive/v0.6.0.tar.gz"],
# )

# Rule repository, note that it's recommended to use a pinned commit to a released version of the rules

# git_repository(
#   name = "rules_foreign_cc",

# )

http_archive(
    name = "rules_foreign_cc",
    sha256 = "d54742ffbdc6924f222d2179f0e10e911c5c659c4ae74158e9fe827aad862ac6",
    strip_prefix = "rules_foreign_cc-0.2.0",
    url = "https://github.com/bazelbuild/rules_foreign_cc/archive/0.2.0.tar.gz",
)

load("@rules_foreign_cc//foreign_cc:repositories.bzl", "rules_foreign_cc_dependencies")

http_archive(
    name = "libccd",
    build_file = "//src:BUILD.libccd",
    strip_prefix = "libccd-2.1",
    urls = [
        "https://github.com/danfis/libccd/archive/refs/tags/v2.1.tar.gz",
    ],
)

http_archive(
    name = "boringssl",
    strip_prefix = "boringssl-16100fd5073b3986ca03efa6bbb501c2e061e7e3",
    urls = ["https://github.com/google/boringssl/archive/16100fd5073b3986ca03efa6bbb501c2e061e7e3.zip"],
)

git_repository(
    name = "com_google_asylo",
    remote = "https://github.com/KeplerC/asylo.git",
    commit = "89aa34697144f5b12a9b20da59b795fa702410a1",
)

load(
    "@com_google_asylo//asylo/bazel:asylo_deps.bzl",
    "asylo_deps",
    "asylo_testonly_deps",
)

asylo_deps()

asylo_testonly_deps()

# sgx_deps is only needed if @linux_sgx is used.
load("@com_google_asylo//asylo/bazel:sgx_deps.bzl", "sgx_deps")

sgx_deps()

# remote_deps is only needed if remote backend is used.
load("@com_google_asylo//asylo/bazel:remote_deps.bzl", "remote_deps")

remote_deps()

# grpc_deps is only needed if gRPC is used. Projects using gRPC as an external
# dependency must call both grpc_deps() and grpc_extra_deps().
load("@com_github_grpc_grpc//bazel:grpc_deps.bzl", "grpc_deps")

grpc_deps()

load("@com_github_grpc_grpc//bazel:grpc_extra_deps.bzl", "grpc_extra_deps")

grpc_extra_deps()

http_archive(
  name = "com_google_absl",
  urls = ["https://github.com/abseil/abseil-cpp/archive/refs/tags/20200923.3.zip"],
)
