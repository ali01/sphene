#ifndef ICMP_PACKET_H_7H1D49WV
#define ICMP_PACKET_H_7H1D49WV

#include "fwk/buffer.h"
using Fwk::Buffer;

#include "packet.h"


class ICMPPacket : public Packet {
 public:
  typedef Fwk::Ptr<const ICMPPacket> PtrConst;
  typedef Fwk::Ptr<ICMPPacket> Ptr;

  static Ptr ICMPPacketNew(Buffer::Ptr buffer, unsigned int buffer_offset) {
    return new ICMPPacket(buffer, buffer_offset);
  }

 protected:
  ICMPPacket(Buffer::Ptr buffer, unsigned int buffer_offset)
      : Packet(buffer, buffer_offset) {}
};

#endif
