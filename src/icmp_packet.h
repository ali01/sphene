#ifndef ICMP_PACKET_H_7H1D49WV
#define ICMP_PACKET_H_7H1D49WV

#include "buffer.h"
#include "packet.h"


class ICMPPacket : public Packet {
 public:
  ICMPPacket(Buffer::Ptr buffer, unsigned int buffer_offset)
      : Packet(buffer, buffer_offset) {}
};

#endif
