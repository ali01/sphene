#ifndef PACKET_H_
#define PACKET_H_

#include "buffer.h"

/* Forward declarations. */
class ARPPacket;
class EthernetPacket;
class ICMPPacket;
class IPPacket;


class Packet {
 public:
  Buffer::Ptr buffer() const { return buffer_; }

  class Functor {
    virtual void operator()(ARPPacket*) { }
    virtual void operator()(EthernetPacket*) { }
    virtual void operator()(ICMPPacket*) { }
    virtual void operator()(IPPacket*) { }
  };

 protected:
  Packet(Buffer::Ptr buffer, unsigned int buffer_offset)
      : buffer_(buffer), buffer_offset_(buffer_offset) { }

  /* Data members. */
  Buffer::Ptr buffer_;
  const unsigned int buffer_offset_;
};

#endif
