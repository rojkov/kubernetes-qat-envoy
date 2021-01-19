load(
    "@envoy//bazel:envoy_build_system.bzl",
    "envoy_cc_extension",
    "envoy_cc_library",
    "envoy_package",
    "envoy_proto_library",
)

licenses(["notice"])  # Apache 2

envoy_package()

envoy_cc_library(
    name = "decompressor_lib",
    srcs = ["qatzip_decompressor_impl.cc"],
    hdrs = ["qatzip_decompressor_impl.h"],
    repository = "@envoy",
    deps = [
        "@envoy//include/envoy/compression/decompressor:decompressor_interface",
        "@qatzip",
    ],
)

envoy_proto_library(
    name = "qatzip",
    srcs = ["qatzip.proto"],
)

envoy_cc_extension(
    name = "config",
    srcs = ["config.cc"],
    hdrs = ["config.h"],
    security_posture = "robust_to_untrusted_downstream",
    repository = "@envoy",
    deps = [
        ":decompressor_lib",
		":qatzip_cc_proto",
        "@envoy//source/common/http:headers_lib",
        "@envoy//include/envoy/compression/decompressor:decompressor_config_interface",
        "@envoy//include/envoy/compression/decompressor:decompressor_factory_interface",
    ],
)
