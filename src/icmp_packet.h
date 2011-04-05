#ifndef ICMP_PACKET_H_7H1D49WV
#define ICMP_PACKET_H_7H1D49WV

#include "fwk/buffer.h"

#include "packet.h"


class ICMPPacket : public Packet {
 public:
  typedef Fwk::Ptr<const ICMPPacket> PtrConst;
  typedef Fwk::Ptr<ICMPPacket> Ptr;

  static Ptr ICMPPacketNew(Fwk::Buffer::Ptr buffer,
                           unsigned int buffer_offset) {
    return new ICMPPacket(buffer, buffer_offset);
  }

 protected:
  ICMPPacket(Fwk::Buffer::Ptr buffer, unsigned int buffer_offset)
      : Packet(buffer, buffer_offset) {}
};

#endif
