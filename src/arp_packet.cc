#include "arp_packet.h"

#include <inttypes.h>
#include "ethernet_packet.h"
#include "interface.h"


namespace {

struct ARPHeader {
  uint16_t htype;                        // hardware type
  uint16_t ptype;                        // protocol type
  uint8_t  hlen;                         // hardware address length
  uint8_t  plen;                         // protocol address length
  uint16_t oper;                         // ARP operation
  uint8_t  sha[EthernetAddr::kAddrLen];  // sender hardware address
  uint32_t spa;                          // sender protocol address
  uint8_t  tha[EthernetAddr::kAddrLen];  // target hardware address
  uint32_t tpa;                          // target protocol address
} __attribute__((packed));

}  // namespace


void ARPPacket::operator()(Functor* const f, const Interface::PtrConst iface) {
  (*f)(this, iface);
}
