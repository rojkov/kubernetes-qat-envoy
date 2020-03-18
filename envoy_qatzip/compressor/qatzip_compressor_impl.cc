#include "envoy_qatzip/compressor/qatzip_compressor_impl.h"

#include <memory>

#include "common/common/assert.h"

#include "absl/container/fixed_array.h"

namespace Envoy {
namespace Compressor {

QatzipCompressorImpl::QatzipCompressorImpl() : QatzipCompressorImpl(4096) {}

QatzipCompressorImpl::QatzipCompressorImpl(size_t chunk_size)
    : initialized_{false}, chunk_size_{chunk_size}, avail_out_{chunk_size},
      chunk_char_ptr_(new unsigned char[chunk_size]), session_{}, stream_{} {
  stream_.out = chunk_char_ptr_.get();
}

QatzipCompressorImpl::~QatzipCompressorImpl() {
  if (initialized_) {
    qzEndStream(&session_, &stream_);
    qzTeardownSession(&session_);
    qzClose(&session_);
  }
}

void QatzipCompressorImpl::init(unsigned int compression_level, unsigned int hardware_buffer_size,
                                unsigned int stream_buffer_size,
                                unsigned int input_size_threshold) {
  QzSessionParams_T params;

  ASSERT(initialized_ == false);

  int status = qzGetDefaults(&params);
  RELEASE_ASSERT(status == QZ_OK, "failed to initialize hardware");
  params.comp_lvl = compression_level;
  params.hw_buff_sz = hardware_buffer_size;
  params.strm_buff_sz = stream_buffer_size;
  params.input_sz_thrshold = input_size_threshold;

  status = qzInit(&session_, params.sw_backup);
  RELEASE_ASSERT(status == QZ_OK || status == QZ_DUPLICATE, "failed to initialize hardware");
  status = qzSetupSession(&session_, &params);
  RELEASE_ASSERT(status == QZ_OK || status == QZ_DUPLICATE, "failed to initialize hardware");

  initialized_ = true;
}

void QatzipCompressorImpl::compress(Buffer::Instance& buffer, State state) {

  const uint64_t num_slices = buffer.getRawSlices(nullptr, 0);
  absl::FixedArray<Buffer::RawSlice> slices(num_slices);
  buffer.getRawSlices(slices.begin(), num_slices);

  for (const Buffer::RawSlice& input_slice : slices) {
    avail_in_ = input_slice.len_;
    stream_.in = static_cast<unsigned char*>(input_slice.mem_);

    while (avail_in_ > 0) {
      process(buffer, 0);
    }

    buffer.drain(input_slice.len_);
  }

  if (state == State::Finish) {
    do {
      process(buffer, 1);
    } while (stream_.pending_out > 0);

    const size_t n_output = chunk_size_ - avail_out_;
    if (n_output > 0) {
      buffer.add(static_cast<void*>(chunk_char_ptr_.get()), n_output);
    }
  }
}

void QatzipCompressorImpl::process(Buffer::Instance& output_buffer, unsigned int last) {
  stream_.in_sz = avail_in_;
  stream_.out_sz = avail_out_;
  auto status = qzCompressStream(&session_, &stream_, last);
  // NOTE: stream_.in_sz and stream_.out_sz have changed their semantics after the call
  //       to qzCompressStream(). Despite their name the new values are consumed input
  //       and produced output (not available buffer sizes).
  avail_out_ -= stream_.out_sz;
  avail_in_ -= stream_.in_sz;
  stream_.in = stream_.in + stream_.in_sz;
  stream_.out = stream_.out + stream_.out_sz;
  RELEASE_ASSERT(status == QZ_OK, "");
  if (avail_out_ == 0) {
    // update output and reset context
    output_buffer.add(static_cast<void*>(chunk_char_ptr_.get()), chunk_size_);
    chunk_char_ptr_ = std::make_unique<unsigned char[]>(chunk_size_);
    avail_out_ = chunk_size_;
    stream_.out = chunk_char_ptr_.get();
  }
}

} // namespace Compressor
} // namespace Envoy
