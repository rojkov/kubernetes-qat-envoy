licenses(["notice"])  # Apache 2

QAT_INCLUDE_PATHS = [
	"-I /QAT_lib/quickassist/include",
]

cc_library(
    name = "qatzip",
    srcs = glob([
        "src/*.c",
        "src/*h",
        "include/*.h",
    ]),
    hdrs = glob(["include/*.h"]),
    copts = QAT_INCLUDE_PATHS,
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
