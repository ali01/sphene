#ifndef UNKNOWN_PACKET_H_MS1LXIP4
#define UNKNOWN_PACKET_H_MS1LXIP4

#include "packet.h"
#include "packet_buffer.h"

class Interface;


class UnknownPacket : public Packet {
 public:
  typedef Fwk::Ptr<const UnknownPacket> PtrConst;
  typedef Fwk::Ptr<UnknownPacket> Ptr;

  static Ptr UnknownPacketNew(PacketBuffer::Ptr buffer,
                              unsigned int buffer_offset) {
    return new UnknownPacket(buffer, buffer_offset);
  }

  virtual bool valid() const { return true; }

  /* Double-dispatch support. */
  virtual void operator()(Functor* f, Fwk::Ptr<const Interface> iface);

 protected:
  UnknownPacket(PacketBuffer::Ptr buffer, unsigned int buffer_offset)
      : Packet(buffer, buffer_offset) {}
};

#endif
