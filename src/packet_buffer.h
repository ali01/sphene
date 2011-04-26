#ifndef PACKET_BUFFER_H_
#define PACKET_BUFFER_H_

#include <cstdlib>
#include <cstring>
#include <inttypes.h>

#include "fwk/buffer.h"
#include "fwk/ptr.h"
#include "fwk/ptr_interface.h"


// PacketBuffer creates buffers meant for use with Packets that can grow at the
// front.
class PacketBuffer : public Fwk::PtrInterface<PacketBuffer> {
 public:
  typedef Fwk::Ptr<const PacketBuffer> PtrConst;
  typedef Fwk::Ptr<PacketBuffer> Ptr;

  // Constructs a new PacketBuffer. The 'len' bytes of 'buffer' are copied into
  // the new PacketBuffer. The new PacketBuffer will be of at least 'len' bytes
  // in size.
  static Ptr New(const void* buffer, size_t len) {
    return new PacketBuffer(buffer, len);
  }

  // Constructs a new PacketBuffer with an internal buffer of at least 'len'
  // bytes.
  static Ptr New(size_t len) {
    return new PacketBuffer(len);
  }

  // Guarantees that the internal buffer is at least 'len' bytes in size,
  // growing the internal buffer if necessary. This is useful when prepending a
  // header to the packet inside the buffer.
  void minimumSizeIs(size_t min_len) {
    if (min_len <= size())
      return;  // nothing to do

    // Create a new buffer of the next largest size.
    const size_t new_size = nextPowerOf2(min_len, 512);
    Fwk::Buffer::Ptr new_buf = Fwk::Buffer::BufferNew(new_size);

    // Copy data.
    memcpy(new_buf->data() + new_size - size(), data(), size());

    // Swap buffers.
    buffer_ = new_buf;
  }

  // Returns a pointer to the internal buffer. This pointer may change across
  // non-const calls to this object.
  uint8_t* data() const { return buffer_->data(); }

  // Returns the size of the internal buffer.
  size_t len() const { return buffer_->len(); }
  size_t size() const { return buffer_->len(); }

 protected:
  PacketBuffer(const void* const buffer, const size_t len) {
    const size_t pbuf_len = nextPowerOf2(len, 512);
    buffer_ = Fwk::Buffer::BufferNew(pbuf_len);

    // Copy data to the end of the buffer.
    memcpy(buffer_->data() + pbuf_len - len, buffer, len);
  }

  PacketBuffer(const size_t len) {
    const size_t pbuf_len = nextPowerOf2(len, 512);
    buffer_ = Fwk::Buffer::BufferNew(pbuf_len);
  }

  ~PacketBuffer() { }

  static size_t nextPowerOf2(size_t num, const size_t minimum) {
    size_t p = minimum;
    while (p < num)
      p *= 2;
    return p;
  }

 private:
  void operator=(const PacketBuffer&);
  PacketBuffer(const PacketBuffer&);

  Fwk::Buffer::Ptr buffer_;
};

#endif
