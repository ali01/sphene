#include "ospf_router.h"

#include "interface.h"
#include "ospf_interface_map.h"

OSPFRouter::OSPFRouter(uint32_t router_id, uint32_t area_id) :
  log_(Fwk::Log::LogNew("OSPFRouter")),
  functor_(this),
  router_id_(router_id),
  area_id_(area_id),
  interfaces_(OSPFInterfaceMap::New()) {}

void
OSPFRouter::packetNew(Packet::Ptr pkt, Interface::PtrConst iface) {
  /* double dispatch */
  (*pkt)(&functor_, iface);
}

OSPFInterfaceMap::Ptr
OSPFRouter::interfaceMap() {
  return interfaces_;
}

OSPFInterfaceMap::PtrConst
OSPFRouter::interfaceMap() const {
  return interfaces_;
}


/* OSPFRouter::PacketFunctor */

OSPFRouter::PacketFunctor::PacketFunctor(OSPFRouter* ospf_router)
    : ospf_router_(ospf_router),
      interfaces_(ospf_router->interfaces_.ptr()),
      log_(ospf_router->log_) {}

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
  /* Packet validation. */
  if (!pkt->valid()) {
    DLOG << "Ignoring invalid packet.";
    return;
  }

  if (ospf_router_->areaID() != pkt->areaID()) {
    DLOG << "Ignoring packet: Area ID doesn't match that of this router.";
    return;
  }

  if (iface->subnetMask() != pkt->subnetMask()) {
    DLOG << "Ignoring packet: Subnet mask does not match "
         << "that of receiving interface";
    return;
  }

  // TODO(ali): Interfaces must be manually removed from OSPFRouter if they
  //   cease to exist. One approach could be to make use of notifications.
  OSPFInterfaceDesc::Ptr ifd;
  ifd = interfaces_->interfaceDesc(iface->ip());

  if (ifd == NULL) {
    // TODO(ali): Could just drop packet here.
    //   OSPFRouter could just assume that interfaces
    //   are always configured externally.
    /* Packet was received on an interface that the dynamic router
     * was unaware about -- possibly a newly created virtual interface.
     * Creating a new interface description object and
     * adding it to the neighbor map. */
    ifd = OSPFInterfaceDesc::New(iface, kDefaultHelloInterval);
    interfaces_->interfaceDescIs(ifd);
  }

  if (ifd->helloint() != pkt->helloint()) {
    DLOG << "Ignoring packet: helloint does not match that of "
         << "receiving interface";
    return;
  }

  uint32_t neighbor_id = pkt->routerID();
  OSPFNeighbor::Ptr neighbor = ifd->neighbor(neighbor_id);

  if (neighbor == NULL) {
    /* Packet was sent by a new neighbor.
     * Creating neighbor object and adding it to the interface description */
    IPPacket::Ptr ip_pkt = Ptr::st_cast<IPPacket>(pkt->enclosingPacket());
    IPv4Addr neighbor_addr = ip_pkt->src();
    neighbor = OSPFNeighbor::New(neighbor_id, neighbor_addr);
    ifd->neighborIs(neighbor);
  }

  /* Refresh neighbor's age. */
  neighbor->ageIs(0);
}

void
OSPFRouter::PacketFunctor::operator()(OSPFLSUPacket* pkt,
                                      Interface::PtrConst iface) {

}

void
OSPFRouter::PacketFunctor::operator()(OSPFLSUAdvertisement* pkt,
                                      Interface::PtrConst iface) {

}

