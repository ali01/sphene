#include "ospf_daemon.h"

#include "fwk/deque.h"
#include "fwk/log.h"
#include "fwk/scoped_lock.h"

#include "control_plane.h"
#include "data_plane.h"
#include "ethernet_packet.h"
#include "ip_packet.h"
#include "ospf_constants.h"
#include "ospf_interface.h"
#include "ospf_interface_map.h"
#include "ospf_packet.h"
#include "ospf_router.h"
#include "packet_buffer.h"
#include "task.h"
#include "time_types.h"


/* Static global log instance */
static Fwk::Log::Ptr log_ = Fwk::Log::LogNew("OSPFDaemon");


OSPFDaemon::Ptr
OSPFDaemon::New(ControlPlane::Ptr cp, DataPlane::Ptr dp) {
  return new OSPFDaemon(cp, dp);
}

OSPFDaemon::OSPFDaemon(ControlPlane::Ptr cp, DataPlane::Ptr dp)
    : PeriodicTask("OSPFDaemon"),
      control_plane_(cp),
      data_plane_(dp),
      ospf_router_(cp->ospfRouter()),
      router_reactor_(RouterReactor::New(this)),
      latest_lsu_(0) {
  ospf_router_->notifieeIs(router_reactor_);
}

void
OSPFDaemon::run() {
  /* Removing links to neighbors who haven't sent a HELLO packet in more
     than three times their advertised HELLOINT. */
  timeout_neighbor_links();

  /* Removing nodes for whom no LSU has been received */
  timeout_topology_entries();

  /* Sending HELLO packets to all connected neighbors
     every iface.HELLOINT seconds. */
  broadcast_timed_hello();

  /* Timed flood of link-state updates. */
  flood_timed_lsu();
}

void
OSPFDaemon::timeSinceLSUIs(Seconds delta) {
  latest_lsu_ = ::time(NULL) - delta.value();
}

/* OSPFDaemon private helper functions. */

void
OSPFDaemon::timeout_topology_entries() {
  OSPFTopology::Ptr topology = ospf_router_->topology();
  OSPFTopology::const_iterator it;
  for (it = topology->nodesBegin(); it != topology->nodesEnd(); ++it) {
    OSPFNode::Ptr node = it->second;
    timeout_node_topology_entries(node);
  }

  topology->onPossibleUpdate();
}

void
OSPFDaemon::timeout_node_topology_entries(OSPFNode::Ptr node) {
  if (node->isPassiveEndpoint()) {
    ELOG << "timeout_node_topology_entries: Attempt to timeout links "
         << "associated with a non-OSPF endpoint";
    return;
  }

  RouterID nd_id = node->routerID();
  Fwk::Deque<OSPFLink::Ptr> del_links;

  for (OSPFNode::const_lna_iter it = node->activeLinksBegin();
       it != node->activeLinksEnd(); ++it) {
    OSPFLink::Ptr link = it->second;
    RouterID peer_id = link->node()->routerID();

    if (ospf_router_->routerID() == peer_id
        && ospf_router_->interfaceMap()->activeGateway(nd_id) != NULL) {
      /* Do not remove link if NODE is directly connected to this router.
         HELLO protocol takes precedence. */
      continue;
    }

    if (link->timeSinceLSU() > OSPF::kDefaultLinkStateUpdateTimeout) {
      ILOG << "Timeout: Active link between " << nd_id << " and "
           << peer_id;
      del_links.pushFront(link);
    }
  }

  for (OSPFNode::const_lnp_iter it = node->passiveLinksBegin();
       it != node->passiveLinksEnd(); ++it) {
    OSPFLink::Ptr link = it->second;
    if (link->timeSinceLSU() > OSPF::kDefaultLinkStateUpdateTimeout){
      ILOG << "Timeout: Passive link between " << nd_id << " and subnet "
           << link->subnet();
      del_links.pushFront(link);
    }
  }

  for (OSPFNode::const_lnp_iter it = node->passiveLinksBegin();
       it != node->passiveLinksEnd(); ++it) {
    OSPFLink::Ptr link = it->second;
    if (link->timeSinceLSU() > OSPF::kDefaultLinkStateUpdateTimeout)
      del_links.pushFront(link);
  }

  Fwk::Deque<OSPFLink::Ptr>::const_iterator del_it;
  for (del_it = del_links.begin(); del_it != del_links.end(); ++del_it) {
    OSPFLink::Ptr link = *del_it;
    if (link->nodeIsPassiveEndpoint())
      node->passiveLinkDel(link->subnet(), link->subnetMask(), false);
    else
      node->activeLinkDel(link->nodeRouterID(), false);
  }
}

void
OSPFDaemon::timeout_neighbor_links() {
  OSPFInterfaceMap::PtrConst ifaces = ospf_router_->interfaceMap();
  OSPFInterfaceMap::const_if_iter if_it = ifaces->ifacesBegin();
  for (; if_it != ifaces->ifacesEnd(); ++if_it) {
    OSPFInterface::Ptr iface = if_it->second;
    timeout_interface_neighbor_links(iface);
  }

  /* Signal possible change to router's link state. */
  ospf_router_->onLinkStateUpdate();
}

void
OSPFDaemon::timeout_interface_neighbor_links(OSPFInterface::Ptr iface) {
  Fwk::Deque<OSPFGateway::Ptr> del_gw;

  OSPFInterface::const_gwa_iter gw_it = iface->activeGatewaysBegin();
  for (; gw_it != iface->activeGatewaysEnd(); ++gw_it) {
    OSPFGateway::Ptr gw = gw_it->second;
    if (gw->timeSinceHello() > 3 * iface->helloint()) {
      /* No HELLO packet has been received from this neighbor in more than
         three times its advertised HELLOINT. Remove from topology and
         trigger LSU flood. */
      del_gw.pushFront(gw);
    }
  }

  Fwk::Deque<OSPFGateway::Ptr>::const_iterator del_it;
  for (del_it = del_gw.begin(); del_it != del_gw.end(); ++del_it) {
    OSPFGateway::Ptr gw = *del_it;

    RouterID nbr_id = gw->nodeRouterID();
    ILOG << "Timeout: Active gateway with subnet " << gw->subnet() << " to nbr "
         << nbr_id;

    iface->activeGatewayDel(nbr_id);
  }
}

void
OSPFDaemon::broadcast_timed_hello() {
  OSPFInterfaceMap::PtrConst iface_map = ospf_router_->interfaceMap();
  OSPFInterfaceMap::const_if_iter if_it = iface_map->ifacesBegin();

  bool hello_logged = false;
  for (; if_it != iface_map->ifacesEnd(); ++if_it) {
    OSPFInterface::Ptr iface = if_it->second;
    if (iface->timeSinceOutgoingHello() >= iface->helloint()) {
      if (!hello_logged) {
        ILOG << "Sending HELLO";
        hello_logged = true;
      }

      broadcast_hello_out_interface(iface);
    }
  }
}

void
OSPFDaemon::broadcast_hello_out_interface(OSPFInterface::Ptr iface) {
  size_t ip_pkt_len = OSPFHelloPacket::kPacketSize + IPPacket::kHeaderSize;
  size_t eth_pkt_len = ip_pkt_len + EthernetPacket::kHeaderSize;

  PacketBuffer::Ptr buffer = PacketBuffer::New(eth_pkt_len);

  OSPFHelloPacket::Ptr ospf_pkt =
    OSPFHelloPacket::NewDefault(buffer,
                                ospf_router_->routerID(),
                                ospf_router_->areaID(),
                                iface->interfaceSubnetMask(),
                                iface->helloint());

  IPPacket::Ptr ip_pkt =
    IPPacket::NewDefault(buffer, ip_pkt_len,
                         IPPacket::kOSPF,
                         iface->interfaceIP(),
                         OSPFHelloPacket::kBroadcastAddr);

  /* IPPacket fields: */
  ip_pkt->ttlIs(1);
  ip_pkt->checksumReset();

  EthernetPacket::Ptr eth_pkt =
    EthernetPacket::New(buffer, buffer->size() - eth_pkt_len);

  /* EthernetPacket fields: */
  eth_pkt->srcIs(iface->interface()->mac());
  eth_pkt->dstIs(EthernetAddr::kBroadcast);
  eth_pkt->typeIs(EthernetPacket::kIP);

  /* Setting enclosing packet. */
  ospf_pkt->enclosingPacketIs(ip_pkt);
  ip_pkt->enclosingPacketIs(eth_pkt);

  /* Sending packet. */
  data_plane_->outputPacketNew(eth_pkt, iface->interface());

  /* Resetting time since last HELLO. */
  iface->timeSinceOutgoingHelloIs(0);
}

void
OSPFDaemon::flood_timed_lsu() {
  if (timeSinceLSU() >= OSPF::kDefaultLinkStateInterval) {
    ospf_router_->onLinkStateInterval();

    /* Resetting time since last LSU. */
    timeSinceLSUIs(0);
  }
}
