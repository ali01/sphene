#include "ospf_router.h"

#include "interface.h"
#include "ip_packet.h"
#include "ospf_interface_map.h"
#include "ospf_node.h"
#include "ospf_packet.h"
#include "ospf_topology.h"

/* OSPFRouter */

OSPFRouter::OSPFRouter(uint32_t router_id, uint32_t area_id) :
  log_(Fwk::Log::LogNew("OSPFRouter")),
  functor_(this),
  router_id_(router_id),
  area_id_(area_id),
  interfaces_(OSPFInterfaceMap::New()),
  topology_(OSPFTopology::New()),
  router_node_(OSPFNode::New(router_id)) {}

void
OSPFRouter::packetNew(Packet::Ptr pkt, Interface::PtrConst iface) {
  /* double dispatch */
  (*pkt)(&functor_, iface);
}

OSPFInterfaceMap::PtrConst
OSPFRouter::interfaceMap() const {
  return interfaces_;
}

OSPFTopology::PtrConst
OSPFRouter::topology() const {
  return topology_;
}

/* OSPFRouter::PacketFunctor */

OSPFRouter::PacketFunctor::PacketFunctor(OSPFRouter* ospf_router)
    : ospf_router_(ospf_router),
      interfaces_(ospf_router->interfaces_.ptr()),
      topology_(ospf_router->topology_.ptr()),
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
    DLOG << "Ignoring invalid OSPF Hello packet.";
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
  /* Packet validation. */
  if (!pkt->valid()) {
    DLOG << "Ignoring invalid OSPF LSU packet.";
    return;
  }

  uint32_t node_id = pkt->routerID();
  if (node_id == ospf_router_->routerID()) {
    /* Drop link-state updates that were originally sent by this router.
     * This will happen if the packet traverses a cycle that leads back to
     * this router. This can be expected to happen relatively often;
     * therefore, no logging is performed. */
    return;
  }

  OSPFNode::Ptr node = topology_->node(node_id);
  if (node) {
    if (node->latestSeqno() >= pkt->seqno()) {
      /* Drop packets with old sequence numbers. */
      return;
    }

  } else {
    /* Creating new node and inserting it into the topology database */
    IPPacket::Ptr ip_pkt = Ptr::st_cast<IPPacket>(pkt->enclosingPacket());
    node = OSPFNode::New(node_id, ip_pkt->src());
    topology_->nodeIs(node);
  }

  /* Updating seqno and the node entry's age in topology database */
  node->latestSeqnoIs(pkt->seqno());
  node->ageIs(0);

  ospf_router_->process_lsu_advertisements(node, pkt);

  // TODO(ali): flood LSU packet.
  // TODO(ali): update the routing table.
  // TODO(ali): deal with contradicting advertisements.
}

/* OSPFRouter private member functions */

void
OSPFRouter::process_lsu_advertisements(OSPFNode::Ptr node,
                                       OSPFLSUPacket::PtrConst pkt) {
  /* Processing each LSU advertisement enclosed in the LSU packet. */
  OSPFLSUAdvertisement::PtrConst adv;
  OSPFNode::Ptr neighbor;
  for (uint32_t adv_index = 0; adv_index < pkt->advCount(); ++adv_index) {
    adv = pkt->advertisement(adv_index);
    neighbor = topology_->node(adv->routerID());
    if (neighbor == NULL) {
      neighbor = OSPFNode::New(adv->routerID());
      topology_->nodeIs(neighbor);
    }

    neighbor->subnetIs(adv->subnet());
    neighbor->subnetMaskIs(adv->subnetMask());

    /* Establish bi-directional neighbor relationship. */
    node->neighborIs(neighbor);
  }
}
