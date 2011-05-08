#include "ospf_router.h"

#include "fwk/log.h"
#include "fwk/scoped_lock.h"

#include "control_plane.h"
#include "interface.h"
#include "ip_packet.h"
#include "ospf_gateway.h"
#include "ospf_interface_map.h"
#include "ospf_link.h"
#include "ospf_node.h"
#include "ospf_packet.h"
#include "ospf_topology.h"
#include "routing_table.h"

/* Static global log instance */
static Fwk::Log::Ptr log_ = Fwk::Log::LogNew("OSPFRouter");


/* OSPFRouter */

OSPFRouter::OSPFRouter(const RouterID& router_id, const AreaID& area_id,
                       RoutingTable::Ptr rtable, Fwk::Ptr<ControlPlane> cp)
    : functor_(this),
      router_id_(router_id),
      area_id_(area_id),
      router_node_(OSPFNode::New(router_id)),
      interfaces_(OSPFInterfaceMap::New()),
      topology_(OSPFTopology::New(router_node_)),
      topology_reactor_(TopologyReactor::New(this)),
      lsu_seqno_(0),
      routing_table_(rtable),
      control_plane_(cp.ptr()) {
  topology_->notifieeIs(topology_reactor_);
}

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

RoutingTable::PtrConst
OSPFRouter::routingTable() const {
  return routing_table_;
}

RoutingTable::Ptr
OSPFRouter::routingTable() {
  return routing_table_;
}

/* OSPFRouter::PacketFunctor */

OSPFRouter::PacketFunctor::PacketFunctor(OSPFRouter* ospf_router)
    : ospf_router_(ospf_router),
      router_node_(ospf_router->router_node_.ptr()),
      interfaces_(ospf_router->interfaces_.ptr()),
      topology_(ospf_router->topology_.ptr()) {}

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
  OSPFInterface::Ptr ifd;
  ifd = interfaces_->interface(iface->ip());

  if (ifd == NULL) {
    /* Packet was received on an interface that the dynamic router
     * was unaware about -- possibly a newly created virtual interface.
     * Creating a new interface description object and
     * adding it to the neighbor map. */
    ifd = OSPFInterface::New(iface, kDefaultHelloInterval);
    interfaces_->interfaceIs(ifd);
  }

  if (ifd->helloint() != pkt->helloint()) {
    DLOG << "Ignoring packet: helloint does not match that of "
         << "receiving interface";
    return;
  }

  RouterID neighbor_id = pkt->routerID();
  OSPFNode::Ptr neighbor = ifd->neighbor(neighbor_id);

  if (neighbor == NULL) {
    /* Packet was sent by a new neighbor.
     * Creating neighbor object and adding it to the interface description */
    IPPacket::Ptr ip_pkt = Ptr::st_cast<IPPacket>(pkt->enclosingPacket());
    IPv4Addr gateway = ip_pkt->src();
    IPv4Addr subnet_mask = pkt->subnetMask();
    IPv4Addr subnet = gateway & subnet_mask;

    neighbor = OSPFNode::New(neighbor_id);
    ifd->gatewayIs(neighbor, gateway, subnet, subnet_mask);

    // TODO(ali): this may need to be a deep copy of link.
    router_node_->linkIs(neighbor, subnet, subnet_mask);
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

  RouterID node_id = pkt->routerID();
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
    node = OSPFNode::New(node_id);
    topology_->nodeIs(node);
  }

  /* Updating seqno and the node entry's age in topology database */
  node->latestSeqnoIs(pkt->seqno());
  node->ageIs(0);

  ospf_router_->process_lsu_advertisements(node, pkt);
  topology_->onUpdate();

  if (pkt->ttl() > 1) {
    pkt->ttlDec(1);
    pkt->checksumReset();
    ospf_router_->forward_lsu_flood(pkt);
  }
}


/* OSPFRouter::NeighborRelationship */

OSPFRouter::NeighborRelationship::NeighborRelationship(OSPFNode::Ptr lsu_sender,
                                                       OSPFLink::Ptr adv_nb)
    : lsu_sender_(lsu_sender), advertised_neighbor_(adv_nb) {}

OSPFRouter::NeighborRelationship::Ptr
OSPFRouter::NeighborRelationship::New(OSPFNode::Ptr lsu_sender,
                                      OSPFLink::Ptr adv_nb) {
  return new NeighborRelationship(lsu_sender, adv_nb);
}

OSPFNode::PtrConst
OSPFRouter::NeighborRelationship::lsuSender() const {
  return lsu_sender_;
}

OSPFNode::Ptr
OSPFRouter::NeighborRelationship::lsuSender() {
  return lsu_sender_;
}

OSPFLink::PtrConst
OSPFRouter::NeighborRelationship::advertisedNeighbor() const {
  return advertised_neighbor_;
}

OSPFLink::Ptr
OSPFRouter::NeighborRelationship::advertisedNeighbor() {
  return advertised_neighbor_;
}

/* OSPFRouter private member functions */

void
OSPFRouter::outputPacketNew(OSPFPacket::Ptr ospf_pkt) {
  IPPacket::Ptr ip_pkt = Ptr::st_cast<IPPacket>(ospf_pkt->enclosingPacket());
  control_plane_->outputPacketNew(ip_pkt);
}

void
OSPFRouter::rtable_update() {
  Fwk::ScopedLock<RoutingTable> lock(routing_table_);

  /* Clear all dynamic entries in the routing table
     before inserting the new ones. */
  routing_table_->clearDynamicEntries();

  OSPFNode::PtrConst next_hop, dest;
  OSPFTopology::const_iterator it;
  for (it = topology_->nodesBegin(); it != topology_->nodesEnd(); ++it) {
    RouterID node_id = it->first;

    next_hop = topology_->nextHop(node_id);
    if (next_hop == NULL)
      continue;

    dest = it->second;
    rtable_add_dest(next_hop, dest);
  }
}

/* Function assumes that routing_table_ is already locked. */
void
OSPFRouter::rtable_add_dest(OSPFNode::PtrConst next_hop,
                            OSPFNode::PtrConst dest) {
  RouterID next_hop_id = next_hop->routerID();
  OSPFInterface::Ptr iface = interfaces_->interface(next_hop_id);
  if (iface == NULL) {
    ELOG << "rtable_add_dest: NEXT_HOP is not connected to any interface.";
    return;
  }

  OSPFNode::const_nb_iter it;
  for (it = dest->neighborsBegin(); it != dest->neighborsEnd(); ++it) {
    OSPFNode::Ptr neighbor = it->second;
    if (dest->prev() == NULL || dest->prev() == neighbor) {
      /* Don't add the subnet through which we are connected to DEST. */
      continue;
    }

    RouterID nbr_id = neighbor->routerID();
    OSPFGateway::Ptr nbr_gw = iface->gateway(nbr_id);

    /* Setting entry's subnet, subnet mask, outgoing interface, and gateway. */
    RoutingTable::Entry::Ptr entry = RoutingTable::Entry::New();
    entry->subnetIs(nbr_gw->subnet(), nbr_gw->subnetMask());
    entry->interfaceIs(iface->interface());
    entry->gatewayIs(nbr_gw->gateway());

    routing_table_->entryIs(entry);
  }
}

void
OSPFRouter::process_lsu_advertisements(OSPFNode::Ptr sender,
                                       OSPFLSUPacket::PtrConst pkt) {
  /* Processing each LSU advertisement enclosed in the LSU packet. */
  OSPFLSUAdvertisement::PtrConst adv;
  OSPFNode::Ptr neighbor_nd;
  for (uint32_t adv_index = 0; adv_index < pkt->advCount(); ++adv_index) {
    adv = pkt->advertisement(adv_index);

    /* Check if the advertised neighbor has also advertised connectivity to
       SENDER. If it has, then there will exist a NeighborRelationship object
       in the LINKS_STAGED multimap. */
    NeighborRelationship::Ptr nb_rel =
      staged_nbr(adv->routerID(), sender->routerID());
    if (nb_rel) {
      /* Staged NeighborRelationship object exists. If the subnets advertised
         for both endpoints of the link, then the neighbor relationship can be
         committed to the router's network topology. */

      OSPFLink::Ptr sender_staged = nb_rel->advertisedNeighbor();
      if (adv->subnet() == sender_staged->subnet()) {
        /* Subnets match. The advertised neighbor relationship is valid. */
        commit_nbr(nb_rel);
      }

    } else {

      /* The advertised neighbor is added to the network topology. The
         neighbor relationship between NEIGHBOR and SENDER, however, is
         staged for now. It is committed when NEIGHBOR confirms connectivity
         to SENDER with an LSU of its own. */

      neighbor_nd = topology_->node(adv->routerID());
      if (neighbor_nd == NULL) {
        neighbor_nd = OSPFNode::New(adv->routerID());
        topology_->nodeIs(neighbor_nd);
      }

      OSPFLink::Ptr neighbor =
        OSPFLink::New(neighbor_nd, adv->subnet(), adv->subnetMask());
      nb_rel = NeighborRelationship::New(sender, neighbor);
      stage_nbr(nb_rel);
    }
  }
}

OSPFLSUPacket::Ptr
OSPFRouter::build_lsu_to_neighbor(OSPFInterface::Ptr iface,
                                  const RouterID& nbr_id) const {
  if (iface->neighbor(nbr_id) == NULL) {
    ELOG << "send_new_lsu_to_neighbor: Node with specified NBR_ID is not "
         << "directly connected to IFACE.";
    return NULL;
  }

  size_t adv_count = interfaces_->gateways();

  size_t ospf_pkt_len = OSPFLSUPacket::kHeaderSize +
                        adv_count * OSPFLSUAdvertisement::kSize;
  size_t ip_pkt_len = IPPacket::kHeaderSize + ospf_pkt_len;
  size_t eth_pkt_len = EthernetPacket::kHeaderSize + ip_pkt_len;

  PacketBuffer::Ptr buffer = PacketBuffer::New(eth_pkt_len);

  /* OSPFPacket. */
  OSPFLSUPacket::Ptr ospf_pkt =
    OSPFLSUPacket::NewDefault(buffer, routerID(), areaID(),
                              adv_count, lsu_seqno_);

  OSPFInterface::const_gw_iter gw_it = iface->gatewaysBegin();
  for (uint32_t ix = 0; gw_it != iface->gatewaysEnd(); ++gw_it, ++ix) {
    OSPFGateway::Ptr gw = gw_it->second;
    OSPFLSUAdvertisement::Ptr adv = ospf_pkt->advertisement(ix);
    adv->routerIDIs(gw->node()->routerID());
    adv->subnetIs(gw->subnet());
    adv->subnetMaskIs(gw->subnetMask());
  }

  /* IPPacket. */
  OSPFGateway::Ptr gw_obj = iface->gateway(nbr_id);
  IPPacket::Ptr ip_pkt =
    IPPacket::NewDefault(buffer, ip_pkt_len, IPPacket::kOSPF,
                         iface->interfaceIP(), gw_obj->gateway());
  ip_pkt->ttlIs(1);
  ip_pkt->checksumReset();

  /* Setting enclosing packet. */
  ospf_pkt->enclosingPacketIs(ip_pkt);

  return ospf_pkt;
}

void
OSPFRouter::forward_lsu_flood(OSPFLSUPacket::Ptr pkt) const {
  RouterID sender_id = pkt->routerID();

  OSPFInterfaceMap::const_if_iter if_it = interfaces_->ifacesBegin();
  for (; if_it != interfaces_->ifacesEnd(); ++if_it) {
    OSPFInterface::PtrConst iface = if_it->second;
    OSPFInterface::const_nb_iter nbr_it = iface->neighborsBegin();
    for (; nbr_it != iface->neighborsEnd(); ++nbr_it) {
      OSPFNode::Ptr nbr = nbr_it->second;
      RouterID nbr_id = nbr->routerID();
      if (nbr_id != sender_id)
        forward_pkt_to_neighbor(nbr_id, pkt);
    }
  }
}

OSPFRouter::NeighborRelationship::Ptr
OSPFRouter::staged_nbr(const RouterID& lsu_sender_id,
                       const RouterID& adv_nb_id) {
  LinkedList<NeighborRelationship>::Ptr nb_list =
    links_staged_.elem(lsu_sender_id);

  if (nb_list) {
    NeighborRelationship::Ptr nbr;
    for (nbr = nb_list->front(); nbr != NULL; nbr = nbr->next()) {
      OSPFNode::PtrConst adv_nb = nbr->advertisedNeighbor()->node();
      if (adv_nb->routerID() == adv_nb_id)
        return nbr;
    }
  }

  return NULL;
}

OSPFRouter::NeighborRelationship::PtrConst
OSPFRouter::staged_nbr(const RouterID& lsu_sender_id,
                       const RouterID& adv_nb_id) const {
  OSPFRouter* self = const_cast<OSPFRouter*>(this);
  return self->staged_nbr(lsu_sender_id, adv_nb_id);
}

bool
OSPFRouter::stage_nbr(OSPFRouter::NeighborRelationship::Ptr nbr) {
  if (nbr == NULL)
    return false;

  RouterID lsu_sender_id = nbr->lsuSender()->routerID();
  RouterID adv_nb_id = nbr->advertisedNeighbor()->node()->routerID();
  if (staged_nbr(lsu_sender_id, adv_nb_id) != NULL) {
    /* NeighborRelationship is already staged. */
    return false;
  }

  LinkedList<NeighborRelationship>::Ptr nb_list =
    links_staged_.elem(lsu_sender_id);
  if (nb_list == NULL) {
    nb_list = LinkedList<NeighborRelationship>::New();
    links_staged_[lsu_sender_id] = nb_list;
  }

  nb_list->pushBack(nbr);

  return true;
}

void
OSPFRouter::commit_nbr(OSPFRouter::NeighborRelationship::Ptr nbr) {
  if (nbr == NULL)
    return;

  OSPFNode::Ptr lsu_sender = nbr->lsuSender();
  OSPFLink::Ptr adv_nb = nbr->advertisedNeighbor();

  /* Establish bi-directional link. */
  lsu_sender->linkIs(adv_nb->node(),
                     adv_nb->subnet(),
                     adv_nb->subnetMask());

  /* Add both nodes to the topology if they weren't already there. */
  topology_->nodeIs(lsu_sender);
  topology_->nodeIs(adv_nb->node());

  /* Unstage neighbor relationship. */
  unstage_nbr(nbr);
}

bool
OSPFRouter::unstage_nbr(OSPFRouter::NeighborRelationship::Ptr nbr) {
  if (nbr == NULL)
    return false;

  RouterID lsu_sender_id = nbr->lsuSender()->routerID();
  LinkedList<NeighborRelationship>::Ptr nb_list =
    links_staged_.elem(lsu_sender_id);

  if (nb_list && nb_list->del(nbr))
    return true;

  return false;
}

void
OSPFRouter::forward_pkt_to_neighbor(const RouterID& neighbor_id,
                                    OSPFPacket::Ptr pkt) const {
  OSPFInterface::PtrConst iface = interfaces_->interface(neighbor_id);
  if (iface == NULL) {
    ELOG << "send_pkt_to_node: Node with NEIGHBOR_ID is not directly connected "
         << "to this router";
    return;
  }

  IPv4Addr src_addr = iface->interfaceIP();
  OSPFGateway::PtrConst nbr_gw = iface->gateway(neighbor_id);
  IPv4Addr dst_addr = nbr_gw->gateway();

  IPPacket::Ptr ip_pkt = Ptr::st_cast<IPPacket>(pkt->enclosingPacket());
  ip_pkt->srcIs(src_addr);
  ip_pkt->dstIs(dst_addr);
  ip_pkt->ttlIs(1);
  ip_pkt->checksumReset();

  control_plane_->outputPacketNew(ip_pkt);
}
