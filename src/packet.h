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
class Interface;
class IPPacket;
class OSPFPacket;
class OSPFHelloPacket;
class OSPFLSUPacket;
class UnknownPacket;


class Packet : public Fwk::PtrInterface<Packet> {
 public:
  typedef Fwk::Ptr<const Packet> PtrConst;
  typedef Fwk::Ptr<Packet> Ptr;

  class Functor {
   public:
    virtual void operator()(ARPPacket*, Fwk::Ptr<const Interface>) { }
    virtual void operator()(EthernetPacket*, Fwk::Ptr<const Interface>) { }
    virtual void operator()(ICMPPacket*, Fwk::Ptr<const Interface>) { }
    virtual void operator()(IPPacket*, Fwk::Ptr<const Interface>) { }
    virtual void operator()(OSPFPacket*, Fwk::Ptr<const Interface>) { }
    virtual void operator()(OSPFHelloPacket*, Fwk::Ptr<const Interface>) { }
    virtual void operator()(OSPFLSUPacket*, Fwk::Ptr<const Interface>) { }
    virtual void operator()(UnknownPacket*, Fwk::Ptr<const Interface>) { }

    virtual ~Functor() { }
  };

  uint8_t* data() const { return buffer_->data() + buffer_offset_; }
  size_t len() const { return buffer_->size() - buffer_offset_; }

  Fwk::Buffer::Ptr buffer() const { return buffer_; }
  unsigned int bufferOffset() const { return buffer_offset_; }

  /* Double-dispatch support. */
  virtual void operator()(Functor* f, Fwk::Ptr<const Interface> iface) = 0;

  /* Returns the enclosing packet. */
  Ptr enclosingPacket() const { return encl_pkt_; }

  /* Sets the enclosing packet. */
  void enclosingPacketIs(Ptr pkt) { encl_pkt_ = pkt; }

 protected:
  Packet(Fwk::Buffer::Ptr buffer, unsigned int buffer_offset)
      : buffer_(buffer), buffer_offset_(buffer_offset), encl_pkt_(NULL) { }

  uint8_t* offsetAddress(unsigned int offset) const;

  /* Data members. */
  Fwk::Buffer::Ptr buffer_;
  const unsigned int buffer_offset_;
  Ptr encl_pkt_;
};

#endif
