#include "arp_packet.h"

#include "interface.h"


void ARPPacket::operator()(Functor* const f, const Interface::PtrConst iface) {
  (*f)(this, iface);
}

