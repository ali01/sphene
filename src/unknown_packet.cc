#include "unknown_packet.h"

#include "interface.h"


void UnknownPacket::operator()(Functor* const f,
                               const Interface::PtrConst iface) {
  (*f)(this, iface);
}
