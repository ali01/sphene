#ifndef ARP_PACKET_H_WDIREBCF
#define ARP_PACKET_H_WDIREBCF

#include "buffer.h"
#include "packet.h"


class ARPPacket : public Packet {
 public:
  ARPPacket(Buffer::Ptr buffer, unsigned int buffer_offset)
      : Packet(buffer, buffer_offset) {}
};

#endif
