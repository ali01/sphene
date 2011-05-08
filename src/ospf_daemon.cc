#include "ospf_daemon.h"

#include "fwk/log.h"
#include "fwk/scoped_lock.h"

#include "control_plane.h"
#include "data_plane.h"
#include "ethernet_packet.h"
#include "ip_packet.h"
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
OSPFDaemon::New(OSPFRouter::Ptr ospf_router,
                ControlPlane::Ptr cp,
                DataPlane::Ptr dp) {
  return new OSPFDaemon(ospf_router, cp, dp);
}

OSPFDaemon::OSPFDaemon(OSPFRouter::Ptr ospf_router,
                       ControlPlane::Ptr cp,
                       DataPlane::Ptr dp)
    : PeriodicTask("OSPFDaemon"),
      control_plane_(cp),
      data_plane_(dp),
      ospf_router_(ospf_router) {}

void
OSPFDaemon::run() {
  /* Sending HELLO packets to all connected neighbors
     every iface.HELLOINT seconds. */
  broadcast_timed_hello();
}

void
OSPFDaemon::timeSinceLSUIs(Seconds delta) {
  latest_lsu_ = ::time(NULL) - delta.value();
}

void
OSPFDaemon::broadcast_timed_hello() {
  OSPFInterfaceMap::PtrConst iface_map = ospf_router_->interfaceMap();
  OSPFInterfaceMap::const_if_iter if_it = iface_map->ifacesBegin();
  for (; if_it != iface_map->ifacesEnd(); ++if_it) {
    OSPFInterface::Ptr iface = if_it->second;
    if (iface->timeSinceHello() > iface->helloint()) {
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

  /* Sending packet. */
  data_plane_->outputPacketNew(eth_pkt, iface->interface());

  /* Resetting time since last HELLO. */
  iface->timeSinceHelloIs(0);
}
