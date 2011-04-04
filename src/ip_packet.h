#ifndef IP_PACKET_H_8CU5IVY3
#define IP_PACKET_H_8CU5IVY3

#include "fwk/buffer.h"

#include "packet.h"


class IPPacket : public Packet {
 public:
  IPPacket(Buffer::Ptr buffer, unsigned int buffer_offset)
      : Packet(buffer, buffer_offset) {}
};

#endif
