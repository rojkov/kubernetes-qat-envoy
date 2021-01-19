#include "qatzip/decompressor/qatzip_decompressor_impl.h"

#include "common/common/assert.h"

namespace Envoy {
namespace Extensions {
namespace Compression {
namespace Qatzip {
namespace Decompressor {

QatzipDecompressorImpl::QatzipDecompressorImpl(QzSession_T* session, size_t chunk_size)
  : chunk_size_{chunk_size}, avail_out_{chunk_size_},
    chunk_char_ptr_{new unsigned char[chunk_size]},
    session_{session}, stream_{} {
  RELEASE_ASSERT(session_ != nullptr,
                 "QATzip compressor must be created with non-null QATzip session");
  stream_.out = chunk_char_ptr_.get();
}

QatzipDecompressorImpl::~QatzipDecompressorImpl() { qzEndStream(session_, &stream_); }

void QatzipDecompressorImpl::decompress(const Buffer::Instance& input_buffer, Buffer::Instance& output_buffer) {
  for (const Buffer::RawSlice& input_slice : input_buffer.getRawSlices()) {
    avail_in_ = input_slice.len_;
    stream_.in = static_cast<unsigned char*>(input_slice.mem_);

    while (avail_in_ > 0) {
      process(output_buffer, 0);
    }
  }

  do {
    process(output_buffer, 1);
  } while (stream_.pending_out > 0);

  const size_t n_output = chunk_size_ - avail_out_;
  if (n_output > 0) {
    output_buffer.add(static_cast<void*>(chunk_char_ptr_.get()), n_output);
  }
}

bool QatzipDecompressorImpl::process(Buffer::Instance& output_buffer, unsigned int last) {
  stream_.in_sz = avail_in_;
  stream_.out_sz = avail_out_;
  auto status = qzDecompressStream(session_, &stream_, last);
  if (status != QZ_OK) {
    printf("OOoooops\n");
    return false;
  }
  // NOTE: stream_.in_sz and stream_.out_sz have changed their semantics after the call
  //       to qzDecompressStream(). Despite their name the new values are consumed input
  //       and produced output (not available buffer sizes).
  avail_out_ -= stream_.out_sz;
  avail_in_ -= stream_.in_sz;
  stream_.in = stream_.in + stream_.in_sz;
  stream_.out = stream_.out + stream_.out_sz;
  if (avail_out_ == 0) {
    // The chunk is full, so copy it to the output buffer and reset context.
    output_buffer.add(static_cast<void*>(chunk_char_ptr_.get()), chunk_size_);
    chunk_char_ptr_ = std::make_unique<unsigned char[]>(chunk_size_);
    avail_out_ = chunk_size_;
    stream_.out = chunk_char_ptr_.get();
  }

  return true;
}

} // namespace Decompressor
} // namespace Qatzip
} // namespace Compression
} // namespace Extensions
} // namespace Envoy
