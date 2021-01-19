#pragma once

#include "common/http/headers.h"

#include "envoy/compression/decompressor/config.h"
#include "envoy/compression/decompressor/factory.h"
#include "qatzip/decompressor/qatzip.pb.h"
#include "qatzip/decompressor/qatzip.pb.validate.h"


//#include "extensions/compression/common/decompressor/factory_base.h"
#include "qatzip/decompressor/qatzip_decompressor_impl.h"

namespace Envoy {
namespace Extensions {
namespace Compression {
namespace Qatzip {
namespace Decompressor {

namespace {
const std::string& qatzipStatsPrefix() { CONSTRUCT_ON_FIRST_USE(std::string, "qatzip."); }
const std::string& qatzipExtensionName() {
  CONSTRUCT_ON_FIRST_USE(std::string, "envoy.compression.qatzip.decompressor");
}

} // namespace

class QatzipDecompressorFactory : public Envoy::Compression::Decompressor::DecompressorFactory {
public:
  QatzipDecompressorFactory(const qatzip::decompressor::Qatzip& qatzip, Server::Configuration::FactoryContext& context);

  // Envoy::Compression::Decompressor::DecompressorFactory
  Envoy::Compression::Decompressor::DecompressorPtr
  createDecompressor(const std::string& stats_prefix) override;
  const std::string& statsPrefix() const override { return qatzipStatsPrefix(); }
  const std::string& contentEncoding() const override {
    return Http::CustomHeaders::get().ContentEncodingValues.Gzip;
  }

private:
  // TODO: the struct is used in compressor - make it common.
  struct QatzipThreadLocal : public ThreadLocal::ThreadLocalObject {
    QatzipThreadLocal(QzSessionParams_T params);
    virtual ~QatzipThreadLocal();
    QzSession_T* getSession();

    QzSessionParams_T params_;
    QzSession_T session_;
    bool initialized_;
  };

  const uint32_t chunk_size_;
  ThreadLocal::SlotPtr tls_slot_;
};

class QatzipDecompressorLibraryFactory
    : public Envoy::Compression::Decompressor::NamedDecompressorLibraryConfigFactory {
public:
  QatzipDecompressorLibraryFactory() : name_{qatzipExtensionName()} {}

  Envoy::Compression::Decompressor::DecompressorFactoryPtr
  createDecompressorFactoryFromProto(const Protobuf::Message& proto_config,
                                   Server::Configuration::FactoryContext& context) override {
    return createDecompressorFactoryFromProtoTyped(
        MessageUtil::downcastAndValidate<const qatzip::decompressor::Qatzip&>(proto_config,
                                                             context.messageValidationVisitor()), context);
  }

  ProtobufTypes::MessagePtr createEmptyConfigProto() override {
    return std::make_unique<qatzip::decompressor::Qatzip>();
  }

  std::string name() const override { return name_; }

private:
  Envoy::Compression::Decompressor::DecompressorFactoryPtr createDecompressorFactoryFromProtoTyped(
      const qatzip::decompressor::Qatzip& config, Server::Configuration::FactoryContext& context);

  const std::string name_;
};

DECLARE_FACTORY(QatzipDecompressorLibraryFactory);

} // namespace Decompressor
} // namespace Qatzip
} // namespace Compression
} // namespace Extensions
} // namespace Envoy
