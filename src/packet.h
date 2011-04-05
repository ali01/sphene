#ifndef PACKET_H_
#define PACKET_H_

#include "fwk/buffer.h"

using Fwk::Buffer;

/* Forward declarations. */
class ARPPacket;
class EthernetPacket;
class ICMPPacket;
class IPPacket;


class Packet {
 public:
  class Functor {
   public:
    virtual void operator()(ARPPacket*) { }
    virtual void operator()(EthernetPacket*) { }
    virtual void operator()(ICMPPacket*) { }
    virtual void operator()(IPPacket*) { }

    virtual ~Functor() { }
  };

  Buffer::Ptr buffer() const { return buffer_; }

 protected:
  Packet(Buffer::Ptr buffer, unsigned int buffer_offset)
      : buffer_(buffer), buffer_offset_(buffer_offset) { }

  uint8_t* offsetAddress(unsigned int offset) const;

  /* Data members. */
  Buffer::Ptr buffer_;
  const unsigned int buffer_offset_;
};

#endif
