#ifndef UNKNOWN_PACKET_H_MS1LXIP4
#define UNKNOWN_PACKET_H_MS1LXIP4

#include "fwk/buffer.h"

#include "packet.h"


class UnknownPacket : public Packet {
 public:
  typedef Fwk::Ptr<const UnknownPacket> PtrConst;
  typedef Fwk::Ptr<UnknownPacket> Ptr;

  static Ptr UnknownPacketNew(Fwk::Buffer::Ptr buffer,
                              unsigned int buffer_offset) {
    return new UnknownPacket(buffer, buffer_offset);
  }

 protected:
  UnknownPacket(Fwk::Buffer::Ptr buffer, unsigned int buffer_offset)
      : Packet(buffer, buffer_offset) {}
};

#endif
