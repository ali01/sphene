#ifndef ARP_PACKET_H_WDIREBCF
#define ARP_PACKET_H_WDIREBCF

#include "fwk/buffer.h"

#include "packet.h"

class Interface;


class ARPPacket : public Packet {
 public:
  typedef Fwk::Ptr<const ARPPacket> PtrConst;
  typedef Fwk::Ptr<ARPPacket> Ptr;

  static Ptr ARPPacketNew(Fwk::Buffer::Ptr buffer,
                          unsigned int buffer_offset) {
    return new ARPPacket(buffer, buffer_offset);
  }

  virtual void operator()(Functor* f, Fwk::Ptr<const Interface> iface);

 protected:
  ARPPacket(Fwk::Buffer::Ptr buffer, unsigned int buffer_offset)
      : Packet(buffer, buffer_offset) { }
};

#endif
