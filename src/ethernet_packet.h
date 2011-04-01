#ifndef ETHERNET_PACKET_H_5KIE50BV
#define ETHERNET_PACKET_H_5KIE50BV

#include "buffer.h"
#include "packet.h"


class EthernetPacket : public Packet {
 public:
  EthernetPacket(Buffer::Ptr buffer, unsigned int buffer_offset)
      : Packet(buffer, buffer_offset) {}
};

#endif
