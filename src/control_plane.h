#ifndef CONTROL_PLANE_H_
#define CONTROL_PLANE_H_

#include <string>

#include "arp_cache.h"
#include "arp_queue.h"
#include "data_plane.h"
#include "icmp_packet.h"
#include "fwk/log.h"
#include "fwk/named_interface.h"
#include "fwk/ptr.h"
#include "interface.h"
#include "interface_map.h"
#include "packet.h"
#include "routing_table.h"
#include "tunnel.h"
#include "tunnel_map.h"

// Forward declarations.
class ARPPacket;
class EthernetPacket;
class GREPacket;
class IPPacket;
class OSPFRouter;
class UnknownPacket;


class ControlPlane : public Fwk::NamedInterface {
 public:
  typedef Fwk::Ptr<const ControlPlane> PtrConst;
  typedef Fwk::Ptr<ControlPlane> Ptr;

  static Ptr ControlPlaneNew() {
    return new ControlPlane("ControlPlane");
  }

  void packetNew(Packet::Ptr pkt, Interface::PtrConst iface);

  // Sends out the given 'ip_packet'. Makes ARP requests and updates the ARP
  // cache if necessary.
  void outputPacketNew(Fwk::Ptr<IPPacket> ip_packet);

  // Send an IP-in-Ethernet packet with the specified parameters.
  void outputRawPacketNew(Fwk::Ptr<IPPacket> ip_packet,
                          Interface::PtrConst out_iface,
                          IPv4Addr next_hop_ip,
                          EthernetAddr dst_mac_addr);

  // Returns the DataPlane.
  DataPlane::Ptr dataPlane() const { return dp_; }

  // Sets the DataPlane.
  void dataPlaneIs(DataPlane::Ptr dp);

  ARPCache::Ptr arpCache() const { return arp_cache_; }
  ARPQueue::Ptr arpQueue() const { return arp_queue_; }

  RoutingTable::Ptr routingTable() const { return routing_table_; }

  Fwk::Ptr<const OSPFRouter> ospfRouter() const;
  Fwk::Ptr<OSPFRouter> ospfRouter();

  // Returns the associated TunnelMap.
  TunnelMap::Ptr tunnelMap() const { return tunnel_map_; }

 protected:
  ControlPlane(const std::string& name);

 private:
  class PacketFunctor : public Packet::Functor {
   public:
    PacketFunctor(ControlPlane* cp);

    void operator()(ARPPacket*, Interface::PtrConst);
    void operator()(EthernetPacket*, Interface::PtrConst);
    void operator()(GREPacket*, Interface::PtrConst);
    void operator()(ICMPPacket*, Interface::PtrConst);
    void operator()(IPPacket*, Interface::PtrConst);
    void operator()(OSPFPacket*, Interface::PtrConst);
    void operator()(UnknownPacket*, Interface::PtrConst);

   private:
    ControlPlane* cp_;
    Fwk::Log::Ptr log_;
  };

  void sendARPRequestAndEnqueuePacket(IPv4Addr next_hop_ip,
                                      Interface::PtrConst out_iface,
                                      IPPacket::Ptr pkt);
  void sendEnqueued(IPv4Addr ip_addr, EthernetAddr eth_addr);
  void updateARPCacheMapping(IPv4Addr ip_addr, EthernetAddr eth_addr);

  void sendICMPEchoReply(IPPacket::Ptr echo_request);
  void sendICMPTTLExceeded(IPPacket::Ptr orig_pkt);
  void sendICMPDestNetworkUnreach(IPPacket::PtrConst orig_pkt);
  void sendICMPDestHostUnreach(IPPacket::PtrConst orig_pkt);
  void sendICMPDestProtoUnreach(IPPacket::PtrConst orig_pkt);
  void sendICMPDestUnreachFragRequired(IPPacket::PtrConst orig_pkt);
  void sendICMPDestUnreach(ICMPPacket::Code code, IPPacket::PtrConst orig_pkt);
  void encapsulateAndOutputPacket(IPPacket::Ptr pkt,
                                  Interface::PtrConst out_iface);

  // Fragments an IP packet and sends the fragments individually.
  void fragmentAndSend(IPPacket::Ptr pkt);

  Fwk::Log::Ptr log_;
  PacketFunctor functor_;
  ARPCache::Ptr arp_cache_;
  ARPQueue::Ptr arp_queue_;
  RoutingTable::Ptr routing_table_;
  Fwk::Ptr<OSPFRouter> ospf_router_;
  TunnelMap::Ptr tunnel_map_;
  DataPlane::Ptr dp_;

  // Operations disallowed.
  ControlPlane(const ControlPlane&);
  void operator=(const ControlPlane&);

  friend class ARPQueueDaemon;
};

#endif
