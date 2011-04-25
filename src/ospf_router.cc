#include "ospf_router.h"

#include "interface.h"

OSPFRouter::OSPFRouter() :
  log_(Fwk::Log::LogNew("OSPFRouter")),
  functor_(this) {}

void
OSPFRouter::packetNew(Packet::Ptr pkt, Interface::PtrConst iface) {
  /* double dispatch */
  (*pkt)(&functor_, iface);
}


/* OSPFRouter::PacketFunctor */

OSPFRouter::PacketFunctor::PacketFunctor(OSPFRouter* ospf_router)
    : ospf_router_(ospf_router), log_(ospf_router->log_) {}

void
OSPFRouter::PacketFunctor::operator()(OSPFPacket* pkt,
                                      Interface::PtrConst iface) {
  /* Double dispatch on more specific OSPFPacket. */
  OSPFPacket::Ptr derived_pkt = pkt->derivedInstance();
  (*derived_pkt)(this, iface);
}

void
OSPFRouter::PacketFunctor::operator()(OSPFHelloPacket* pkt,
                                      Interface::PtrConst iface) {

}

void
OSPFRouter::PacketFunctor::operator()(OSPFLSUAdvertisement* pkt,
                                      Interface::PtrConst iface) {

}


void
OSPFRouter::PacketFunctor::operator()(OSPFLSUPacket* pkt,
                                      Interface::PtrConst iface) {

}
