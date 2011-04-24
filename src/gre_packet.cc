#include "gre_packet.h"

#include <arpa/inet.h>
#include <inttypes.h>
#include "ethernet_packet.h"
#include "fwk/buffer.h"
#include "interface.h"
#include "ip_packet.h"


const size_t GREPacket::kHeaderSize = sizeof(struct GREHeader);


GREPacket::GREPacket(Fwk::Buffer::Ptr buffer, unsigned int buffer_offset)
    : Packet(buffer, buffer_offset),
      gre_hdr_((struct GREHeader *)offsetAddress(0)) { }


void GREPacket::operator()(Functor* const f, const Interface::PtrConst iface) {
  (*f)(this, iface);
}
