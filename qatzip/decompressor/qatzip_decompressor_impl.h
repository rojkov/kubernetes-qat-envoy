#pragma once

#include "envoy/compression/decompressor/decompressor.h"

#include "qatzip.h"

namespace Envoy {
namespace Extensions {
namespace Compression {
namespace Qatzip {
namespace Decompressor {

/**
 * Implementation of decompressor's interface.
 */
class QatzipDecompressorImpl : public Envoy::Compression::Decompressor::Decompressor {
public:
  QatzipDecompressorImpl(QzSession_T* session, size_t chunk_size);
  ~QatzipDecompressorImpl() override;

  // Compression::Decompressor::Decompressor
  void decompress(const Buffer::Instance& input_buffer, Buffer::Instance& output_buffer, const bool last) override;

private:
  bool process(Buffer::Instance& output_buffer, unsigned int last);

  const size_t chunk_size_;
  size_t avail_in_;
  size_t avail_out_;

  std::unique_ptr<unsigned char[]> chunk_char_ptr_;
  QzSession_T* const session_;
  QzStream_T stream_;
};

} // namespace Decompressor
} // namespace Qatzip
} // namespace Compression
} // namespace Extensions
} // namespace Envoy
