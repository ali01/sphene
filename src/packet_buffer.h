#ifndef PACKET_BUFFER_H_QNX4VITL
#define PACKET_BUFFER_H_QNX4VITL

#include "fwk/ptr_interface.h"

class PacketBuffer : public Fwk::PtrInterface<PacketBuffer> {
 public:
  typedef Fwk::Ptr<const PacketBuffer> PtrConst;
  typedef Fwk::Ptr<PacketBuffer> Ptr;

  static Ptr PacketBufferNew(void *buffer, size_t len) {
    return new PacketBuffer(buffer, len);
  }

 private:
  PacketBuffer(void *_buffer, size_t _len) : buffer_(_buffer), len_(_len) {}

  /* data members */
  void *buffer_;
  size_t len_;

  /* operations disallowed */
  PacketBuffer(const PacketBuffer&);
  void operator=(const PacketBuffer&);
};

#endif
