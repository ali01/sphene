#ifndef PACKET_H_
#define PACKET_H_

#include <inttypes.h>

#include "fwk/buffer.h"
#include "fwk/ptr.h"
#include "fwk/ptr_interface.h"

/* Forward declarations. */
class ARPPacket;
class EthernetPacket;
class ICMPPacket;
class IPPacket;
class UnknownPacket;


class Packet : public Fwk::PtrInterface<Packet> {
 public:
  typedef Fwk::Ptr<const Packet> PtrConst;
  typedef Fwk::Ptr<Packet> Ptr;

  class Functor {
   public:
    virtual void operator()(ARPPacket*) { }
    virtual void operator()(EthernetPacket*) { }
    virtual void operator()(ICMPPacket*) { }
    virtual void operator()(IPPacket*) { }
    virtual void operator()(UnknownPacket*) { }

    virtual ~Functor() { }
  };

  uint8_t* data() const { return buffer_->data() + buffer_offset_; }
  size_t len() const { return buffer_->size() - buffer_offset_; }

  Fwk::Buffer::Ptr buffer() const { return buffer_; }

  /* Double-dispatch support. */
  virtual void operator()(Functor* f) = 0;

 protected:
  Packet(Fwk::Buffer::Ptr buffer, unsigned int buffer_offset)
      : buffer_(buffer), buffer_offset_(buffer_offset) { }

  uint8_t* offsetAddress(unsigned int offset) const;

  /* Data members. */
  Fwk::Buffer::Ptr buffer_;
  const unsigned int buffer_offset_;
};

#endif
