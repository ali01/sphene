#include "ospf_router.h"

#include "interface.h"
#include "ip_packet.h"
#include "ospf_interface_map.h"
#include "ospf_neighbor.h"
#include "ospf_node.h"
#include "ospf_packet.h"
#include "ospf_topology.h"
#include "routing_table.h"

/* OSPFRouter */

OSPFRouter::OSPFRouter(const RouterID& router_id, const AreaID& area_id) :
  log_(Fwk::Log::LogNew("OSPFRouter")),
  functor_(this),
  router_id_(router_id),
  area_id_(area_id),
  router_node_(OSPFNode::New(router_id)),
  interfaces_(OSPFInterfaceMap::New()),
  topology_(OSPFTopology::New(router_node_)),
  routing_table_(NULL) {}

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

void
OSPFRouter::routingTableIs(RoutingTable::Ptr rtable) {
  routing_table_ = rtable;
}

/* OSPFRouter::PacketFunctor */

OSPFRouter::PacketFunctor::PacketFunctor(OSPFRouter* ospf_router)
    : ospf_router_(ospf_router),
      router_node_(ospf_router->router_node_.ptr()),
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

  RouterID neighbor_id = pkt->routerID();
  OSPFNode::Ptr neighbor = ifd->neighbor(neighbor_id);

  if (neighbor == NULL) {
    /* Packet was sent by a new neighbor.
     * Creating neighbor object and adding it to the interface description */
    IPPacket::Ptr ip_pkt = Ptr::st_cast<IPPacket>(pkt->enclosingPacket());
    IPv4Addr neighbor_addr = ip_pkt->src();
    IPv4Addr subnet_mask = pkt->subnetMask();
    IPv4Addr subnet = neighbor_addr & subnet_mask;

    neighbor = OSPFNode::New(neighbor_id);
    ifd->neighborIs(neighbor, subnet, subnet_mask);

    // TODO(ali): this may need to be a deep copy of neighbor.
    router_node_->neighborIs(neighbor, subnet, subnet_mask);
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
    // TODO(ali): make use of new interface
    // IPPacket::Ptr ip_pkt = Ptr::st_cast<IPPacket>(pkt->enclosingPacket());
    node = OSPFNode::New(node_id);
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


/* OSPFRouter::NeighborRelationship */

OSPFRouter::NeighborRelationship::NeighborRelationship(OSPFNode::Ptr lsu_sender,
                                                       OSPFNeighbor::Ptr adv_nb)
    : lsu_sender_(lsu_sender), advertised_neighbor_(adv_nb) {}

OSPFRouter::NeighborRelationship::Ptr
OSPFRouter::NeighborRelationship::New(OSPFNode::Ptr lsu_sender,
                                      OSPFNeighbor::Ptr adv_nb) {
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

OSPFNeighbor::PtrConst
OSPFRouter::NeighborRelationship::advertisedNeighbor() const {
  return advertised_neighbor_;
}

OSPFNeighbor::Ptr
OSPFRouter::NeighborRelationship::advertisedNeighbor() {
  return advertised_neighbor_;
}

/* OSPFRouter private member functions */

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

      OSPFNeighbor::Ptr sender_staged = nb_rel->advertisedNeighbor();
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

      OSPFNeighbor::Ptr neighbor =
        OSPFNeighbor::New(neighbor_nd, adv->subnet(), adv->subnetMask());
      nb_rel = NeighborRelationship::New(sender, neighbor);
      stage_nbr(nb_rel);
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
  OSPFNeighbor::Ptr adv_nb = nbr->advertisedNeighbor();

  /* Establish bi-directional neighbor relationship. */
  lsu_sender->neighborIs(adv_nb->node(),
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
