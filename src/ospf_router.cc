#include "ospf_router.h"

#include "fwk/log.h"
#include "fwk/scoped_lock.h"

#include "control_plane.h"
#include "interface.h"
#include "interface_map.h"
#include "ip_packet.h"
#include "ospf_constants.h"
#include "ospf_endpoint.h"
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

OSPFRouter::OSPFRouter(const RouterID& router_id,
                       const AreaID& area_id,
                       RoutingTable::Ptr rtable,
                       InterfaceMap::Ptr iface_map,
                       ControlPlane* cp)
    : area_id_(area_id),
      router_node_(OSPFNode::New(router_id)),
      interfaces_(OSPFInterfaceMap::New(iface_map)),
      topology_(OSPFTopology::New(router_node_)),
      topology_reactor_(TopologyReactor::New(this)),
      im_reactor_(OSPFInterfaceMapReactor::New(this)),
      rtable_reactor_(RoutingTableReactor::New(this)),
      lsu_seqno_(0),
      lsu_dirty_(true),
      routing_table_(rtable),
      control_plane_(cp),
      functor_(this) {
  topology_->notifieeIs(topology_reactor_);
  interfaces_->notifieeIs(im_reactor_);
  rtable_reactor_->notifierIs(routing_table_);

  /* Process existing routes in RTABLE. */
  RoutingTable::const_iterator it;
  for (it = routing_table_->entriesBegin();
       it != routing_table_->entriesEnd();
       ++it) {
    RoutingTable::Entry::Ptr entry = it->second;
    rtable_reactor_->onEntry(routing_table_, entry);
  }
}

void
OSPFRouter::packetNew(OSPFPacket::Ptr pkt, Interface::PtrConst iface) {
  /* double dispatch */
  (*pkt)(&functor_, iface);
}

OSPFInterfaceMap::PtrConst
OSPFRouter::interfaceMap() const {
  return interfaces_;
}

OSPFInterfaceMap::Ptr
OSPFRouter::interfaceMap() {
  return interfaces_;
}

OSPFTopology::PtrConst
OSPFRouter::topology() const {
  return topology_;
}

OSPFTopology::Ptr
OSPFRouter::topology() {
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

void
OSPFRouter::onLinkStateUpdate() {
  if (lsu_dirty_) {
    flood_lsu();
    lsu_dirty_ = false;
  }
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
  DLOG << "OSPFHelloPacket dispatch in OSPFRouter.";

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

  OSPFInterface::Ptr ifd;
  ifd = interfaces_->interface(iface->ip());

  if (ifd == NULL) {
    /* Packet was received on an interface that the dynamic router
     * was unaware about -- possibly a newly created virtual interface.
     * Creating a new interface description object and
     * adding it to the neighbor map. */
    ifd = OSPFInterface::New(iface, OSPF::kDefaultHelloInterval);
    interfaces_->interfaceIs(ifd);
  }

  if (ifd->helloint() != pkt->helloint()) {
    DLOG << "Ignoring packet: helloint does not match that of "
         << "receiving interface";
    return;
  }

  RouterID neighbor_id = pkt->routerID();
  OSPFGateway::Ptr gw_obj = ifd->gateway(neighbor_id);

  if (gw_obj == NULL) {
    /* Packet was sent by a new neighbor.
     * Creating neighbor object and adding it to the interface description */
    IPPacket::Ptr ip_pkt = Ptr::st_cast<IPPacket>(pkt->enclosingPacket());
    IPv4Addr gateway = ip_pkt->src();
    IPv4Addr subnet_mask = pkt->subnetMask();
    IPv4Addr subnet = gateway & subnet_mask;

    OSPFNode::Ptr neighbor = OSPFNode::New(neighbor_id);
    gw_obj = OSPFGateway::New(neighbor, gateway, subnet, subnet_mask);
    ifd->gatewayIs(gw_obj);
  }

  /* Refresh neighbor's time since last HELLO. */
  gw_obj->timeSinceHelloIs(0);
}

void
OSPFRouter::PacketFunctor::operator()(OSPFLSUPacket* pkt,
                                      Interface::PtrConst iface) {
  DLOG << "OSPFLSUPacket dispatch in OSPFRouter.";

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

    DLOG << "Inserted node " << node_id << " to topology database.";
  }

  /* Updating seqno in topology database */
  node->latestSeqnoIs(pkt->seqno());

  ospf_router_->process_lsu_advertisements(node, pkt);
  topology_->onPossibleUpdate();

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

/* OSPFRouter::OSPFInterfaceMapReactor */

void
OSPFRouter::OSPFInterfaceMapReactor::onInterface(OSPFInterfaceMap::Ptr _im,
                                                 const IPv4Addr& addr) {
  /* router ID should always be equal to the IP addr of the first interface. */
  if (ospf_router_->routerID() == OSPF::kInvalidRouterID
      || _im->interfaces() <= 1) {
    ospf_router_->routerIDIs((RouterID)addr.value());
  }

  /* Process any unprocessed static routes that correspond to
     the added interface. */
  OSPFInterface::Ptr iface = _im->interface(addr);
  RoutingTable::Ptr rtable = ospf_router_->routingTable();
  RoutingTable::Entry::Ptr entry = rtable->entry(iface->interfaceSubnet(),
                                                 iface->interfaceSubnetMask());
  if (entry) {
    OSPFGateway::Ptr gw_obj = iface->gateway(entry->gateway());
    if (gw_obj == NULL)
      ospf_router_->rtable_reactor_->onEntry(rtable, entry);
  }
}

void
OSPFRouter::OSPFInterfaceMapReactor::onGateway(OSPFInterfaceMap::Ptr _im,
                                               OSPFInterface::Ptr iface,
                                               const RouterID& nd_id) {
  OSPFGateway::Ptr gw = _im->gateway(nd_id);

  /* Add node to the topology if it wasn't already there. */
  ospf_router_->topology_->nodeIs(gw->node(), false);

  ospf_router_->router_node_->linkIs(gw);
  ospf_router_->lsu_dirty_ = true;
}

void
OSPFRouter::OSPFInterfaceMapReactor::onGatewayDel(OSPFInterfaceMap::Ptr _im,
                                                  OSPFInterface::Ptr iface,
                                                  const RouterID& nd_id) {
  ospf_router_->router_node_->linkDel(nd_id);
  ospf_router_->lsu_dirty_ = true;

  /* Remove node from topology. */
  ospf_router_->topology_->nodeDel(nd_id);
}

/* OSPFRouter::RoutingTableReactor. */

void
OSPFRouter::RoutingTableReactor::onEntry(RoutingTable::Ptr rtable,
                                         RoutingTable::Entry::Ptr entry) {
  if (entry->type() == RoutingTable::Entry::kDynamic)
    return;

  DLOG << "Processing static routing table entry with subnet "
       << entry->subnet();

  OSPFInterfaceMap::Ptr iface_map = ospf_router_->interfaceMap();
  OSPFInterface::Ptr iface = iface_map->interface(entry->interface()->ip());
  if (iface) {
    RouterID endpoint_id = ospf_router_->topology()->routerIDNew();
    OSPFEndpoint::Ptr endpoint_nd = OSPFEndpoint::New(endpoint_id);
    OSPFGateway::Ptr gw_obj = OSPFGateway::New(endpoint_nd,
                                               entry->gateway(),
                                               entry->subnet(),
                                               entry->subnetMask());
    iface->gatewayIs(gw_obj);
  }
}

void
OSPFRouter::RoutingTableReactor::onEntryDel(RoutingTable::Ptr rtable,
                                            RoutingTable::Entry::Ptr entry) {
  if (entry->type() == RoutingTable::Entry::kDynamic)
    return;

  OSPFInterfaceMap::Ptr iface_map = ospf_router_->interfaceMap();
  OSPFInterface::Ptr iface = iface_map->interface(entry->interface()->ip());
  iface->gatewayDel(entry->gateway());
}

/* OSPFRouter private member functions */

void
OSPFRouter::outputPacketNew(OSPFPacket::Ptr ospf_pkt) {
  if (ospf_pkt->type() == OSPFPacket::kLSU)
    ++lsu_seqno_;

  IPPacket::Ptr ip_pkt = Ptr::st_cast<IPPacket>(ospf_pkt->enclosingPacket());
  control_plane_->outputPacketNew(ip_pkt);
}

void
OSPFRouter::rtable_update() {
  DLOG << "Full routing table update.";

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
  RouterID dest_id = dest->routerID();

  OSPFGateway::Ptr gw_obj = interfaces_->gateway(next_hop_id);
  if (gw_obj == NULL) {
    ELOG << "rtable_add_dest: NEXT_HOP is not connected to any interface.";
    return;
  }

  OSPFNode::const_nb_iter it;
  for (it = dest->neighborsBegin(); it != dest->neighborsEnd(); ++it) {
    OSPFNode::Ptr neighbor = it->second;
    if (dest->prev() == neighbor && dest_id != next_hop_id) {
      /* Don't add the subnet through which we are connected to DEST unless
         the next hop to DEST is DEST itself. */
      continue;
    }

    OSPFLink::PtrConst link = dest->link(neighbor->routerID());
    rtable_add_gateway(link->subnet(),
                       link->subnetMask(),
                       gw_obj->gateway(),
                       gw_obj->interface());
  }
}

void
OSPFRouter::rtable_add_gateway(const IPv4Addr& subnet,
                               const IPv4Addr& mask,
                               const IPv4Addr& gateway,
                               OSPFInterface::PtrConst iface) {
  /* Setting entry's subnet, subnet mask, outgoing interface, and gateway. */
  RoutingTable::Entry::Ptr entry = RoutingTable::Entry::New();
  entry->subnetIs(subnet, mask);
  entry->gatewayIs(gateway);
  entry->interfaceIs(iface->interface());

  routing_table_->entryIs(entry);

  DLOG << "Routing table entry added:";
  DLOG << "Subnet:      " << subnet;
  DLOG << "Subnet mask: " << mask;
  DLOG << "Gateway:     " << gateway;
  DLOG << "Interface:   " << iface->interface()->name();
}

void
OSPFRouter::rtable_remove_gateway(OSPFGateway::Ptr gw,
                                  OSPFInterface::Ptr iface) {
  routing_table_->entryDel(gw->subnet(), gw->subnetMask());

  DLOG << "Routing table entry removed:";
  DLOG << "Subnet:      " << gw->subnet();
  DLOG << "Subnet mask: " << gw->subnetMask();
  DLOG << "Gateway:     " << gw->gateway();
  DLOG << "Interface:   " << iface->interface()->name();
}

void
OSPFRouter::process_lsu_advertisements(OSPFNode::Ptr sender,
                                       OSPFLSUPacket::PtrConst pkt) {
  /* Processing each LSU advertisement enclosed in the LSU packet. */
  OSPFLSUAdvertisement::PtrConst adv;
  OSPFNode::Ptr neighbor_nd;
  for (uint32_t adv_index = 0; adv_index < pkt->advCount(); ++adv_index) {
    adv = pkt->advertisement(adv_index);

    NeighborRelationship::Ptr nb_rel;
    if (adv->routerID() != OSPF::kPassiveEndpointID) {
      /* Check if the advertised neighbor has also advertised connectivity to
         SENDER. If it has, then there will exist a NeighborRelationship object
         in the LINKS_STAGED multimap. */
      nb_rel = staged_nbr(adv->routerID(), sender->routerID());
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
          topology_->nodeIs(neighbor_nd, false);
        }

        OSPFLink::Ptr neighbor =
          OSPFLink::New(neighbor_nd, adv->subnet(), adv->subnetMask());
        nb_rel = NeighborRelationship::New(sender, neighbor);
        stage_nbr(nb_rel);
      }

    } else {
      /* Advertisement corresponds to an endpoint that is not running OSPF.
         Bypass two-phase commit logic. */
      RouterID endpoint_id = topology()->routerIDNew();
      OSPFEndpoint::Ptr endpoint_nd = OSPFEndpoint::New(endpoint_id);
      OSPFLink::Ptr link =
        OSPFLink::New(endpoint_nd, adv->subnet(), adv->subnetMask());
      sender->linkIs(link, false);

      /* Add both endpoints to the topology in case they aren't already there */
      topology_->nodeIs(sender, false);
      topology_->nodeIs(link->node(), false);
    }
  }
}

void
OSPFRouter::flood_lsu() {
  OSPFInterfaceMap::const_if_iter if_it = interfaces_->ifacesBegin();
  for (; if_it != interfaces_->ifacesEnd(); ++if_it) {
    OSPFInterface::Ptr iface = if_it->second;
    flood_lsu_out_interface(iface);
  }

  if (notifiee_)
    notifiee_->onLinkStateFlood(this);
}

void
OSPFRouter::flood_lsu_out_interface(Fwk::Ptr<OSPFInterface> iface) {
  OSPFInterface::const_gw_iter it;
  for (it = iface->gatewaysBegin(); it != iface->gatewaysEnd(); ++it) {
    OSPFGateway::Ptr gw_obj = it->second;
    OSPFNode::Ptr nbr = gw_obj->node();
    if (nbr->isEndpoint()){
      /* Do not send link-state updates to non-OSPF endpoints. */
      continue;
    }

    OSPFLSUPacket::Ptr ospf_pkt = build_lsu_to_neighbor(iface, nbr->routerID());

    DLOG << "Sending link-state update to " << nbr->routerID();
    outputPacketNew(ospf_pkt);
  }
}

OSPFLSUPacket::Ptr
OSPFRouter::build_lsu_to_neighbor(OSPFInterface::Ptr iface,
                                  const RouterID& nbr_id) const {
  if (nbr_id == 0 || nbr_id == OSPF::kInvalidRouterID) {
    ELOG << "send_new_lsu_to_neighbor: Attempt to build link-state update to "
            "a node with an invalid router ID";
    return NULL;
  }

  OSPFGateway::Ptr gw_obj = iface->gateway(nbr_id);
  if (gw_obj == NULL) {
    ELOG << "send_new_lsu_to_neighbor: Node with specified NBR_ID is not "
         << "directly connected to IFACE.";
    return NULL;
  }

  OSPFNode::Ptr nbr = gw_obj->node();
  if (nbr->isEndpoint()) {
    ELOG << "send_new_lsu_to_neighbor: Attempt to build link-state update to "
         << "non-OSPF endpoint";
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

  OSPFInterface::const_gw_iter gw_it = interfaces_->gatewaysBegin();
  for (uint32_t ix = 0; gw_it != interfaces_->gatewaysEnd(); ++gw_it, ++ix) {
    OSPFGateway::Ptr gw = gw_it->second;
    OSPFLSUAdvertisement::Ptr adv = ospf_pkt->advertisement(ix);

    /* If gateway peer is a non-OSPF endpoint, the advertised
       router ID must be kPassiveEndpointID. */
    RouterID adv_rid = gw->nodeIsEndpoint() ? OSPF::kPassiveEndpointID
                                                : gw->nodeRouterID();

    adv->routerIDIs(adv_rid);
    adv->subnetIs(gw->subnet());
    adv->subnetMaskIs(gw->subnetMask());
  }

  ospf_pkt->checksumReset();

  /* IPPacket. */
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
    OSPFInterface::const_gw_iter gw_it = iface->gatewaysBegin();
    for (; gw_it != iface->gatewaysEnd(); ++gw_it) {
      OSPFGateway::Ptr gw_obj = gw_it->second;
      OSPFNode::Ptr nbr = gw_obj->node();
      RouterID nbr_id = nbr->routerID();

      if (nbr->isEndpoint()) {
        /* Do not forward link-state update floods to non-OSPF neighbors. */
        continue;
      }

      if (nbr_id == sender_id) {
        /* Do not forward link-state update flood to its original sender. */
        continue;
      }

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

  /* Establish bi-directional link.
     This also refreshes the link's time since last LSU. */
  lsu_sender->linkIs(adv_nb, false);

  /* Add both nodes to the topology if they weren't already there. */
  topology_->nodeIs(lsu_sender, false);
  topology_->nodeIs(adv_nb->node(), false);

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
  if (neighbor_id == 0 || neighbor_id == OSPF::kInvalidRouterID) {
    ELOG << "forward_pkt_to_neighbor: Attempt to forward an OSPF packet to a "
            "neighbor with an invalid router ID.";
    return;
  }

  OSPFInterface::PtrConst iface = interfaces_->interface(neighbor_id);
  if (iface == NULL) {
    ELOG << "send_pkt_to_node: Node with NEIGHBOR_ID is not directly connected "
         << "to this router";
    return;
  }

  OSPFGateway::PtrConst gw_obj = iface->gateway(neighbor_id);
  if (gw_obj->nodeIsEndpoint()) {
    ELOG << "forward_pkt_to_neighbor: Attempt to forward an OSPF packet to a "
         << "non-OSPF neighbor.";
    return;
  }

  IPv4Addr src_addr = iface->interfaceIP();
  IPv4Addr dst_addr = gw_obj->gateway();

  IPPacket::Ptr ip_pkt = Ptr::st_cast<IPPacket>(pkt->enclosingPacket());
  ip_pkt->srcIs(src_addr);
  ip_pkt->dstIs(dst_addr);
  ip_pkt->ttlIs(1);
  ip_pkt->checksumReset();

  control_plane_->outputPacketNew(ip_pkt);
}
