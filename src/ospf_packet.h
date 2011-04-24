#ifndef OSPF_PACKET_H_7R33RO3C
#define OSPF_PACKET_H_7R33RO3C

#include "fwk/buffer.h"

#include "packet.h"

/* Forward declarations. */
struct ospf_pkt;

class OSPFPacket : public Packet {
 public:
  typedef Fwk::Ptr<const OSPFPacket> PtrConst;
  typedef Fwk::Ptr<OSPFPacket> Ptr;

  static const uint8_t kVersion = 2;

  static Ptr New(Fwk::Buffer::Ptr buffer, unsigned int buffer_offset) {
    return new OSPFPacket(buffer, buffer_offset);
  }

  /* Double-dispatch support. */
  // TODO: this may only be necessary in derived classes
  virtual void operator()(Functor* f, Fwk::Ptr<const Interface> iface);

 protected:
  OSPFPacket(Fwk::Buffer::Ptr buffer, unsigned int buffer_offset);

private:
  struct ospf_pkt* ospf_pkt_;
};

#endif
