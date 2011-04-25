#ifndef GRE_PACKET_H_
#define GRE_PACKET_H_

#include <string>

#include "fwk/buffer.h"

#include "packet.h"

class Interface;


class GREPacket : public Packet {
 public:
  typedef Fwk::Ptr<const GREPacket> PtrConst;
  typedef Fwk::Ptr<GREPacket> Ptr;

  static Ptr GREPacketNew(Fwk::Buffer::Ptr buffer,
                          unsigned int buffer_offset) {
    return new GREPacket(buffer, buffer_offset);
  }

  virtual void operator()(Functor* f, Fwk::Ptr<const Interface> iface);

 protected:
  GREPacket(Fwk::Buffer::Ptr buffer, unsigned int buffer_offset);

 private:
  struct GREHeader {
    uint16_t c_resv0_ver;  // C bit, Reserved0, Ver
    uint16_t ptype;        // protocol type
    uint16_t cksum;        // checksum of header and payload (optional)
    uint16_t resv1;        // Reserved1 (optional)
  } __attribute__((packed));

  struct GREHeader* gre_hdr_;
};

#endif
