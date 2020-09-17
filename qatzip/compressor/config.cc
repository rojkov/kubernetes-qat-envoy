#include "qatzip/compressor/config.h"

namespace Envoy {
namespace Extensions {
namespace Compression {
namespace Qatzip {
namespace Compressor {

namespace {
// Default qatzip chunk size.
const uint32_t DefaultChunkSize = 4096;
} // namespace

QatzipCompressorFactory::QatzipCompressorFactory(
    const qatzip::compressor::Qatzip& qatzip)
    : chunk_size_(PROTOBUF_GET_WRAPPED_OR_DEFAULT(qatzip, chunk_size, DefaultChunkSize)) {}

Envoy::Compression::Compressor::CompressorPtr QatzipCompressorFactory::createCompressor() {
  QzSession_T sess{};
  return std::make_unique<QatzipCompressorImpl>(&sess);
}

Envoy::Compression::Compressor::CompressorFactoryPtr
QatzipCompressorLibraryFactory::createCompressorFactoryFromProtoTyped(
    const qatzip::compressor::Qatzip& proto_config) {
  return std::make_unique<QatzipCompressorFactory>(proto_config);
}

/**
 * Static registration for the qatzip compressor library. @see NamedCompressorLibraryConfigFactory.
 */
REGISTER_FACTORY(QatzipCompressorLibraryFactory,
                 Envoy::Compression::Compressor::NamedCompressorLibraryConfigFactory);

} // namespace Compressor
} // namespace Qatzip
} // namespace Compression
} // namespace Extensions
} // namespace Envoy
