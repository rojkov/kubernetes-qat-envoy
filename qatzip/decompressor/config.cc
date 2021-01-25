#include "qatzip/decompressor/config.h"

namespace Envoy {
namespace Extensions {
namespace Compression {
namespace Qatzip {
namespace Decompressor {

namespace {

// Default qatzip chunk size.
const uint32_t DefaultChunkSize = 4096;

} // namespace

QatzipDecompressorFactory::QatzipDecompressorFactory(const qatzip::decompressor::Qatzip& qatzip, Server::Configuration::FactoryContext& context)
  : chunk_size_{PROTOBUF_GET_WRAPPED_OR_DEFAULT(qatzip, chunk_size, DefaultChunkSize)},
    tls_slot_{context.threadLocal().allocateSlot()} {
  QzSessionParams_T params;
  const int status = qzGetDefaults(&params);
  RELEASE_ASSERT(status == QZ_OK, "failed to initialize hardware");

  params.direction = QZ_DIR_DECOMPRESS;
  params.sw_backup = 0;
  //params.data_fmt = QZ_DEFLATE_RAW;
  tls_slot_->set([params](Event::Dispatcher&) -> ThreadLocal::ThreadLocalObjectSharedPtr {
    return std::make_shared<QatzipThreadLocal>(params);
  });
}

Envoy::Compression::Decompressor::DecompressorPtr QatzipDecompressorFactory::createDecompressor(const std::string&) {
  return std::make_unique<QatzipDecompressorImpl>(tls_slot_->getTyped<QatzipThreadLocal>().getSession(), chunk_size_);
}

QatzipDecompressorFactory::QatzipThreadLocal::QatzipThreadLocal(QzSessionParams_T params)
    : params_(params), session_{}, initialized_(false) {}

QatzipDecompressorFactory::QatzipThreadLocal::~QatzipThreadLocal() {
  if (initialized_) {
    qzTeardownSession(&session_);
    qzClose(&session_);
  }
}

QzSession_T* QatzipDecompressorFactory::QatzipThreadLocal::getSession() {
  // The session must be initialized only once in every worker thread.
  if (!initialized_) {

    int status = qzInit(&session_, params_.sw_backup);
    RELEASE_ASSERT(status == QZ_OK || status == QZ_DUPLICATE, "failed to initialize hardware");
    status = qzSetupSession(&session_, &params_);
    RELEASE_ASSERT(status == QZ_OK || status == QZ_DUPLICATE, "failed to setup session");
    initialized_ = true;
  }

  return &session_;
}

Envoy::Compression::Decompressor::DecompressorFactoryPtr
QatzipDecompressorLibraryFactory::createDecompressorFactoryFromProtoTyped(
    const qatzip::decompressor::Qatzip& proto_config, Server::Configuration::FactoryContext& context) {
  return std::make_unique<QatzipDecompressorFactory>(proto_config, context);
}

/**
 * Static registration for the qatzip decompressor library. @see NamedDecompressorLibraryConfigFactory.
 */
REGISTER_FACTORY(QatzipDecompressorLibraryFactory,
                 Envoy::Compression::Decompressor::NamedDecompressorLibraryConfigFactory);

} // namespace Decompressor
} // namespace Qatzip
} // namespace Compression
} // namespace Extensions
} // namespace Envoy
