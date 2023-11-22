load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_library", "cc_test")
load(
    "//:copts.bzl",
    "DEFAULT_COPTS",
    "DEFAULT_LINKOPTS",
)

package(default_visibility= ["//visibility:public"])

cc_library(
    name = "perlin_noise",
    hdrs = ["perlin_noise.h"],
    copts = DEFAULT_COPTS,
    linkopts = DEFAULT_LINKOPTS,
)