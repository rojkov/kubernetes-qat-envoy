licenses(["notice"])  # Apache 2

cc_library(
    name = "host-qat",
    srcs = [
        "libqat_s.so",
        "libusdm_drv_s.so",
    ],
    linkstatic = False,
    visibility = ["//visibility:public"],
)

alias(
    name = "qat",
    actual = "host-qat",
    visibility = ["//visibility:public"],
)
