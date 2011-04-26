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

  // Constructs a new PacketBuffer. The contents of buffer are copied into the
  // new PacketBuffer.
  static Ptr New(const void* buffer, size_t len) {
    return new PacketBuffer(buffer, len);
  }

  // Constructs a new PacketBuffer with an internal buffer of at least len
  // bytes.
  static Ptr New(size_t len) {
    return new PacketBuffer(len);
  }

  uint8_t* data() const { return buffer_->data(); }
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
