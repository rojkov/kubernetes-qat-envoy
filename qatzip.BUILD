licenses(["notice"])  # Apache 2

QAT_DEFINES = [
    "-DADF_PCI_API",
]

cc_library(
    name = "qatzip",
    srcs = glob([
        "src/*.c",
        "src/*h",
        "include/*.h",
    ]),
    hdrs = glob(["include/*.h"]),
    #copts = QAT_DEFINES,
    includes = [
        "include",
        "src",
    ],
    visibility = ["//visibility:public"],
    deps = [
        "@envoy//bazel/foreign_cc:zlib",
        "@qat//:qat",
    ],
)
