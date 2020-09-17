#pragma once

#include "envoy/compression/compressor/factory.h"
#include "qatzip/compressor/qatzip.pb.h"
#include "qatzip/compressor/qatzip.pb.validate.h"

#include "common/http/headers.h"

#include "extensions/compression/common/compressor/factory_base.h"
#include "qatzip/compressor/qatzip_compressor_impl.h"

namespace Envoy {
namespace Extensions {
namespace Compression {
namespace Qatzip {
namespace Compressor {

namespace {

const std::string& qatzipStatsPrefix() { CONSTRUCT_ON_FIRST_USE(std::string, "qatzip."); }
const std::string& qatzipExtensionName() {
  CONSTRUCT_ON_FIRST_USE(std::string, "envoy.compression.qatzip.compressor");
}

} // namespace

class QatzipCompressorFactory : public Envoy::Compression::Compressor::CompressorFactory {
public:
  QatzipCompressorFactory(
      const qatzip::compressor::Qatzip& qatzip);

  // Envoy::Compression::Compressor::CompressorFactory
  Envoy::Compression::Compressor::CompressorPtr createCompressor() override;
  const std::string& statsPrefix() const override { return qatzipStatsPrefix(); }
  const std::string& contentEncoding() const override {
    return Http::CustomHeaders::get().ContentEncodingValues.Gzip;
  }

private:
  const uint32_t chunk_size_;
};

class QatzipCompressorLibraryFactory
    : public Compression::Common::Compressor::CompressorLibraryFactoryBase<
          qatzip::compressor::Qatzip> {
public:
  QatzipCompressorLibraryFactory() : CompressorLibraryFactoryBase(qatzipExtensionName()) {}

private:
  Envoy::Compression::Compressor::CompressorFactoryPtr createCompressorFactoryFromProtoTyped(
      const qatzip::compressor::Qatzip& config) override;
};

DECLARE_FACTORY(QatzipCompressorLibraryFactory);

} // namespace Compressor
} // namespace Qatzip
} // namespace Compression
} // namespace Extensions
} // namespace Envoy
