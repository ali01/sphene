#include "control_plane.h"

#include <string>
#include "fwk/log.h"
#include "fwk/named_interface.h"
#include "fwk/scoped_lock.h"

#include "arp_packet.h"
#include "gre_packet.h"
#include "ethernet_packet.h"
#include "icmp_packet.h"
#include "interface.h"
#include "interface_map.h"
#include "ip_packet.h"
#include "ospf_constants.h"
#include "ospf_packet.h"
#include "ospf_router.h"
#include "packet_buffer.h"
#include "tunnel.h"
#include "tunnel_map.h"

using std::string;


// Input to TCP stack.
void sr_transport_input(uint8_t* packet);


ControlPlane::ControlPlane(const std::string& name)
    : Fwk::NamedInterface(name),
      log_(Fwk::Log::LogNew(name)),
      functor_(this),
      arp_cache_(ARPCache::New()),
      arp_queue_(ARPQueue::New()),
      tunnel_map_(TunnelMap::New()) { }


void ControlPlane::packetNew(Packet::Ptr pkt,
                             const Interface::PtrConst iface) {
  // Dispatch packet using double-dispatch.
  (*pkt)(&functor_, iface);
}


// Assumes the outgoing IP packet is valid (or does not need to be
// valid). Therefore, packets enter this function only if they are not destined
// for the router. ICMP error messages will be generated if the outgoing packet
// cannot be forwarded.
void ControlPlane::outputPacketNew(IPPacket::Ptr pkt) {
  DLOG << "outputPacketNew() in ControlPlane";

  if (pkt->ttl() < 1) {
    // Send ICMP Time Exceeded Message to source.
    pkt->ttlIs(pkt->ttl() + 1);
    sendICMPTTLExceeded(pkt);
    return;
  }

  IPv4Addr dest_ip = pkt->dst();
  Interface::Ptr target_iface;
  {
    InterfaceMap::Ptr iface_map = dp_->interfaceMap();
    Fwk::ScopedLock<InterfaceMap> lock(iface_map);
    target_iface = iface_map->interfaceAddr(dest_ip);
  }

  if (target_iface) {
    DLOG << "outputPacketNew: ignoring outgoing packet destined to this router";
    return;
  }

  // Look up routing table entry of the longest prefix match.
  RoutingTable::Entry::Ptr r_entry;
  {
    RoutingTable::Ptr rtable = dp_->controlPlane()->routingTable();
    Fwk::ScopedLock<RoutingTable> lock(rtable);
    r_entry = rtable->lpm(dest_ip);
  }
  if (!r_entry) {
    DLOG << "No route to " << dest_ip;

    bool send_unreach = true;
    if (pkt->protocol() == IPPacket::kICMP) {
      ICMPPacket::Ptr icmp_pkt((ICMPPacket*)pkt->payload().ptr());

      // Don't generate ICMP when sending ICMP.
      if (icmp_pkt->type() == ICMPPacket::kDestUnreachable) {
        DLOG << "avoiding an ICMP-on-ICMP message";
        send_unreach = false;
      }
    }

    if (send_unreach) {
      // ICMP Destination Host Unreachable.
      sendICMPDestHostUnreach(pkt);
    }

    return;
  }

  // Outgoing interface.
  Interface::PtrConst out_iface = r_entry->interface();
  DLOG << "  outgoing interface: " << out_iface->name();

  if (out_iface->type() == Interface::kVirtual) {
    // Interface is virtual. We need to do an encapsulation here.
    encapsulateAndOutputPacket(pkt, out_iface);
    return;
  }

  // Fragment large datagrams.
  if (pkt->len() > EthernetPacket::kMTU) {
    DLOG << "  len: " << pkt->len() << "; fragmenting";
    fragmentAndSend(pkt);
    return;
  }

  // Next hop IP address.
  IPv4Addr next_hop_ip = r_entry->gateway();
  if (next_hop_ip == 0u)
    next_hop_ip = pkt->dst();
  DLOG << "  next hop: " << next_hop_ip;

  // Look up ARP entry for next hop.
  ARPCache::Entry::Ptr arp_entry;
  {
    ARPCache::Ptr cache = dp_->controlPlane()->arpCache();
    Fwk::ScopedLock<ARPCache> lock(cache);
    arp_entry = cache->entry(next_hop_ip);
  }
  if (!arp_entry) {
    DLOG << "ARP Cache miss for " << string(next_hop_ip);
    sendARPRequestAndEnqueuePacket(next_hop_ip, out_iface, pkt);
    return;
  }

  // Add Ethernet header using ARP entry data.
  pkt->buffer()->minimumSizeIs(pkt->len() + EthernetPacket::kHeaderSize);
  EthernetPacket::Ptr eth_pkt =
      EthernetPacket::New(pkt->buffer(),
                          pkt->bufferOffset() - EthernetPacket::kHeaderSize);
  eth_pkt->srcIs(out_iface->mac());
  eth_pkt->dstIs(arp_entry->ethernetAddr());
  eth_pkt->typeIs(EthernetPacket::kIP);

  if (pkt->protocol() == IPPacket::kTCP) {
    ILOG << "TCP out " << pkt->src() << " -> " << pkt->dst()
         << " via " << out_iface->name();
  } else if (pkt->protocol() == IPPacket::kUDP) {
    ILOG << "UDP out " << pkt->src() << " -> " << pkt->dst()
         << " via " << out_iface->name();
  } else if (pkt->protocol() == IPPacket::kICMP) {
    ILOG << "ICMP out " << pkt->src() << " -> " << pkt->dst()
         << " via " << out_iface->name();
  }

  // Send packet.
  DLOG << "Forwarding IP packet to " << string(next_hop_ip);
  dp_->outputPacketNew(eth_pkt, out_iface);
}


void ControlPlane::outputRawPacketNew(Fwk::Ptr<IPPacket> pkt,
                                      Interface::PtrConst out_iface,
                                      IPv4Addr next_hop_ip,
                                      const EthernetAddr dst_mac_addr) {
  DLOG << "outputRawPacketNew() in ControlPlane";

  if (pkt->ttl() < 1) {
    // Send ICMP Time Exceeded Message to source.
    pkt->ttlIs(pkt->ttl() + 1);
    sendICMPTTLExceeded(pkt);
    return;
  }

  IPv4Addr dest_ip = pkt->dst();
  Interface::Ptr target_iface;
  {
    InterfaceMap::Ptr iface_map = dp_->interfaceMap();
    Fwk::ScopedLock<InterfaceMap> lock(iface_map);
    target_iface = iface_map->interfaceAddr(dest_ip);
  }

  if (target_iface) {
    DLOG << "outputRawPacketNew: ignoring outgoing packet destined "
         << "to this router";
    return;
  }

  DLOG << "  outgoing interface: " << out_iface->name();

  if (out_iface->type() == Interface::kVirtual) {
    // Interface is virtual. We need to do an encapsulation here.
    encapsulateAndOutputPacket(pkt, out_iface);
    return;
  }

  // Fragment large datagrams.
  if (pkt->len() > EthernetPacket::kMTU) {
    DLOG << "  len: " << pkt->len() << "; fragmenting";
    fragmentAndSend(pkt);
    return;
  }

  // Next hop IP address.
  if (next_hop_ip == 0u)
    next_hop_ip = pkt->dst();
  DLOG << "  next hop: " << next_hop_ip;

  // Add Ethernet header using ARP entry data.
  pkt->buffer()->minimumSizeIs(pkt->len() + EthernetPacket::kHeaderSize);
  EthernetPacket::Ptr eth_pkt =
      EthernetPacket::New(pkt->buffer(),
                          pkt->bufferOffset() - EthernetPacket::kHeaderSize);
  eth_pkt->srcIs(out_iface->mac());
  eth_pkt->dstIs(dst_mac_addr);
  eth_pkt->typeIs(EthernetPacket::kIP);

  if (pkt->protocol() == IPPacket::kTCP) {
    ILOG << "TCP out " << pkt->src() << " -> " << pkt->dst()
         << " via " << out_iface->name();
  } else if (pkt->protocol() == IPPacket::kUDP) {
    ILOG << "UDP out " << pkt->src() << " -> " << pkt->dst()
         << " via " << out_iface->name();
  } else if (pkt->protocol() == IPPacket::kICMP) {
    ILOG << "ICMP out " << pkt->src() << " -> " << pkt->dst()
         << " via " << out_iface->name();
  }

  // Send packet.
  DLOG << "Forwarding IP packet to " << string(next_hop_ip);
  dp_->outputPacketNew(eth_pkt, out_iface);
}


void ControlPlane::dataPlaneIs(DataPlane::Ptr dp) {
  if (dp_ == dp)
    return;
  dp_ = dp;

  /* OSPF router ID should be the IP addr of the router's first interface. */
  InterfaceMap::Ptr iface_map = dp->interfaceMap();
  InterfaceMap::const_iterator it = iface_map->begin();
  RouterID rid = OSPF::kInvalidRouterID;
  if (it != iface_map->end()) {
    Interface::Ptr iface = it->second;
    rid = (RouterID)iface->ip().value();
  }

  routing_table_ = RoutingTable::New(iface_map);

  /* Initializing OSPF router. */
  ospf_router_ = OSPFRouter::New(rid, OSPF::kDefaultAreaID,
                                 routing_table_, dp->interfaceMap(), this);
}


OSPFRouter::PtrConst ControlPlane::ospfRouter() const {
  return ospf_router_;
}


OSPFRouter::Ptr ControlPlane::ospfRouter() {
  return ospf_router_;
}


/* ControlPlane::PacketFunctor */

ControlPlane::PacketFunctor::PacketFunctor(ControlPlane* const cp)
    : cp_(cp), log_(cp->log_) { }


void ControlPlane::PacketFunctor::operator()(ARPPacket* const pkt,
                                             const Interface::PtrConst iface) {
  DLOG << "ARPPacket dispatch in ControlPlane";

  if (!pkt->valid()) {
    DLOG << "  packet is invalid; dropping";
    return;
  }

  DLOG << "  operation: " << pkt->operationName();
  DLOG << "  sender HW addr: " << (string)pkt->senderHWAddr();
  DLOG << "  sender IP addr: " << (string)pkt->senderPAddr();
  DLOG << "  target HW addr: " << (string)pkt->targetHWAddr();
  DLOG << "  target IP addr: " << (string)pkt->targetPAddr();

  IPv4Addr sender_ip = pkt->senderPAddr();
  IPv4Addr target_ip = pkt->targetPAddr();
  EthernetAddr sender_eth = pkt->senderHWAddr();
  EthernetAddr target_eth = pkt->targetHWAddr();
  bool merge_flag = false;

  // Update <sender IP, sender MAC> pair in ARP cache if it exists.
  ARPCache::Entry::Ptr cache_entry;
  {
    Fwk::ScopedLock<ARPCache> lock(cp_->arpCache());
    cache_entry = cp_->arpCache()->entry(sender_ip);
  }
  if (cache_entry) {
    cache_entry->ethernetAddrIs(sender_eth);
    cache_entry->ageIs(0);
    merge_flag = true;
  }

  // Are we the target of the ARP packet?
  {
    InterfaceMap::Ptr if_map = cp_->dataPlane()->interfaceMap();
    Fwk::ScopedLock<InterfaceMap> if_map_lock(if_map);
    Interface::Ptr target_iface = if_map->interfaceAddr(target_ip);
    if (!target_iface || target_iface->ip() != target_ip) {
      DLOG << "  ARP packet is not for us; ignoring";
      return;
    }
  }

  // Add <sender IP, sender MAC> to ARP cache.
  if (!merge_flag) {
    cache_entry = ARPCache::Entry::New(sender_ip, sender_eth);
    cache_entry->typeIs(ARPCache::Entry::kDynamic);

    Fwk::ScopedLock<ARPCache> lock(cp_->arpCache());
    cp_->arpCache()->entryIs(cache_entry);
  }

  // Look at the opcode.
  if (pkt->operation() == ARPPacket::kRequest) {
    DLOG << "sending ARP reply";

    // Swap the hardware and protocol fields.
    pkt->targetHWAddrIs(sender_eth);
    pkt->targetPAddrIs(sender_ip);
    pkt->senderHWAddrIs(iface->mac());
    pkt->senderPAddrIs(iface->ip());

    // Flip the operation.
    pkt->operationIs(ARPPacket::kReply);

    // Get the enclosing EthernetPacket.
    EthernetPacket::Ptr eth_pkt =
        (EthernetPacket*)(pkt->enclosingPacket().ptr());

    // Update the source address to our MAC.
    eth_pkt->srcIs(iface->mac());

    // Send ARP reply on same interface.
    cp_->dataPlane()->outputPacketNew(eth_pkt, iface);
  } else if (pkt->operation() == ARPPacket::kReply) {
    IPv4Addr ip_addr = pkt->senderPAddr();
    EthernetAddr eth_addr = pkt->senderHWAddr();

    DLOG << "ARP reply for " << ip_addr << " received";

    cp_->updateARPCacheMapping(ip_addr, eth_addr);
    cp_->sendEnqueued(ip_addr, eth_addr);
  }
}


void ControlPlane::PacketFunctor::operator()(EthernetPacket* const pkt,
                                             const Interface::PtrConst iface) {
  DLOG << "EthernetPacket dispatch in ControlPlane";

  if (!pkt->valid()) {
    DLOG << "  packet is invalid; dropping";
    return;
  }

  // Dispatch encapsulated packet.
  Packet::Ptr payload_pkt = pkt->payload();
  (*payload_pkt)(this, iface);
}


void ControlPlane::PacketFunctor::operator()(GREPacket* const pkt,
                                             const Interface::PtrConst iface) {
  DLOG << "GREPacket dispatch in ControlPlane";

  if (!pkt->valid()) {
    DLOG << "  packet is invalid; dropping";
    return;
  }

  IPPacket::Ptr ip_pkt = Ptr::st_cast<IPPacket>(pkt->enclosingPacket());

  ILOG << "GRE in " << ip_pkt->dst() << " <- " << ip_pkt->src()
       << " on " << iface->name();

  // Do we have a tunnel with this remote?
  Tunnel::Ptr tunnel;
  {
    TunnelMap::Ptr tun_map = cp_->tunnelMap();
    Fwk::ScopedLock<TunnelMap> tun_map_lock(tun_map);
    tunnel = tun_map->tunnelRemoteAddr(ip_pkt->src());
    if (!tunnel) {
      DLOG << "  no GRE tunnel to " << ip_pkt->src() << ", ignoring";
      return;
    }
  }

  // Simulate the packet coming from the tunnel's virtual interface.
  Packet::Ptr payload_pkt = pkt->payload();
  cp_->dataPlane()->packetNew(payload_pkt, tunnel->interface());
}


void ControlPlane::PacketFunctor::operator()(ICMPPacket* const pkt,
                                             const Interface::PtrConst iface) {
  // TODO(ms): Further dispatch of ICMPPacket types should probably be here.
  DLOG << "ICMPPacket dispatch in ControlPlane";

  if (!pkt->valid()) {
    DLOG << "  packet is invalid; dropping";
    return;
  }

  DLOG << "  type: " << pkt->type() << " (" << pkt->typeName() << ")";
  DLOG << "  code: " << (uint32_t)pkt->code();

  IPPacket::Ptr ip_pkt = (IPPacket*)(pkt->enclosingPacket().ptr());

  ILOG << "ICMP in " << ip_pkt->dst() << " <- " << ip_pkt->src()
       << " on " << iface->name();

  // Handle ICMP Echo Requests.
  if (pkt->type() == ICMPPacket::kEchoRequest) {
    cp_->sendICMPEchoReply(ip_pkt);
  }
}


void ControlPlane::PacketFunctor::operator()(IPPacket* const pkt,
                                             const Interface::PtrConst iface) {
  DLOG << "IPPacket dispatch in ControlPlane";

  if (!pkt->valid()) {
    DLOG << "  packet is invalid; dropping";
    return;
  }

  // We don't handle UDP packets at all.
  if (pkt->protocol() == IPPacket::kUDP) {
    DLOG << "  protocol is UDP";
    cp_->sendICMPDestProtoUnreach(pkt);
    return;
  }

  // Send TCP packets to TCP stack.
  if (pkt->protocol() == IPPacket::kTCP) {
    DLOG << "  protocol is TCP";
    sr_transport_input(pkt->data());
    return;
  }

  // Dispatch encapsulated packet.
  Packet::Ptr payload_pkt = pkt->payload();
  (*payload_pkt)(this, iface);
}


void ControlPlane::PacketFunctor::operator()(OSPFPacket* pkt,
                                             Interface::PtrConst iface) {
  DLOG << "OSPFPacket dispatch in ControlPlane";

  if (cp_->ospf_router_)
    cp_->ospf_router_->packetNew(pkt, iface);
  else
    ELOG << "  OSPFRouter is uninitialized: dropping packet.";
}


void ControlPlane::PacketFunctor::operator()(UnknownPacket* const pkt,
                                             const Interface::PtrConst iface) {
  DLOG << "UnknownPacket dispatch in ControlPlane";
}


void
ControlPlane::sendARPRequestAndEnqueuePacket(IPv4Addr next_hop_ip,
                                             Interface::PtrConst out_iface,
                                             IPPacket::Ptr pkt) {
  // Do we already have a packet queue for this next hop in the arp queue?
  ARPQueue::Entry::Ptr entry = arp_queue_->entry(next_hop_ip);
  if (entry == NULL) {
    // No entry in the ARP queue yet.
    size_t pkt_len = EthernetPacket::kHeaderSize + ARPPacket::kHeaderSize;

    PacketBuffer::Ptr buffer = PacketBuffer::New(pkt_len);
    EthernetPacket::Ptr req_eth_pkt =
        EthernetPacket::New(buffer, buffer->size() - pkt_len);

    req_eth_pkt->srcIs(out_iface->mac());
    req_eth_pkt->dstIs(EthernetAddr::kBroadcast);
    req_eth_pkt->typeIs(EthernetPacket::kARP);

    ARPPacket::Ptr arp_pkt = Ptr::st_cast<ARPPacket>(req_eth_pkt->payload());
    arp_pkt->operationIs(ARPPacket::kRequest);
    arp_pkt->hwTypeIs(ARPPacket::kEthernet);
    arp_pkt->pTypeIs(ARPPacket::kIP);
    arp_pkt->senderHWAddrIs(out_iface->mac());
    arp_pkt->senderPAddrIs(out_iface->ip());
    arp_pkt->targetHWAddrIs(EthernetAddr::kZero);
    arp_pkt->targetPAddrIs(next_hop_ip);

    // Send new ARP request immediately.
    DLOG << "Sending ARP request for " << string(next_hop_ip);
    DLOG << "  ARP sender HW addr: " << arp_pkt->senderHWAddr();
    DLOG << "  ARP sender P addr:  " << arp_pkt->senderPAddr();
    DLOG << "  ARP target HW addr: " << arp_pkt->targetHWAddr();
    DLOG << "  ARP target P addr:  " << arp_pkt->targetPAddr();

    dp_->outputPacketNew(req_eth_pkt, out_iface);

    // Create an entry in the ARP queue for the next hop.
    entry = ARPQueue::Entry::New(next_hop_ip, out_iface, req_eth_pkt);
    arp_queue_->entryIs(entry);
  }

  // Add packet that generated this ARP request to the ARP queue.
  DLOG << "queueing packet for " << pkt->dst() << " pending ARP reply";

  // Update Ethernet header using ARP entry.
  pkt->buffer()->minimumSizeIs(pkt->len() + EthernetPacket::kHeaderSize);
  EthernetPacket::Ptr eth_pkt =
      EthernetPacket::New(pkt->buffer(),
                          pkt->bufferOffset() - EthernetPacket::kHeaderSize);
  eth_pkt->typeIs(EthernetPacket::kIP);
  entry->packetIs(eth_pkt);
}


void
ControlPlane::updateARPCacheMapping(IPv4Addr ip_addr, EthernetAddr eth_addr) {
  Fwk::ScopedLock<ARPCache> lock(arp_cache_);
  ARPCache::Entry::Ptr entry = arp_cache_->entry(ip_addr);
  if (entry == NULL) {
    entry = ARPCache::Entry::New(ip_addr, eth_addr);
    arp_cache_->entryIs(entry);
  } else {
    entry->ethernetAddrIs(eth_addr);
    entry->ageIs(0);
  }
}


void
ControlPlane::sendEnqueued(IPv4Addr ip_addr, EthernetAddr eth_addr) {
  DLOG << "Flushing ARP queue for " << ip_addr;

  ARPQueue::Entry::Ptr queue_entry = arp_queue_->entry(ip_addr);
  if (queue_entry) {
    Interface::PtrConst out_iface = queue_entry->interface();
    DLOG << "  out iface: " << out_iface->name();

    ARPQueue::PacketWrapper::Ptr pkt_wrapper = queue_entry->front();
    EthernetPacket::Ptr eth_pkt;
    while (pkt_wrapper != NULL) {
      eth_pkt = pkt_wrapper->packet();
      eth_pkt->srcIs(out_iface->mac());
      eth_pkt->dstIs(eth_addr);

      DLOG << "  forwarding queued packet to " << ip_addr;
      dp_->outputPacketNew(eth_pkt, out_iface);

      pkt_wrapper = pkt_wrapper->next();
    }

    arp_queue_->entryDel(queue_entry);
  }
}


void ControlPlane::sendICMPEchoReply(IPPacket::Ptr ip_pkt) {
  ICMPPacket::Ptr pkt = Ptr::st_cast<ICMPPacket>(ip_pkt->payload());

  // Swap IP src/dst.
  IPv4Addr sender = ip_pkt->src();
  ip_pkt->srcIs(ip_pkt->dst());
  ip_pkt->dstIs(sender);

  // Change type to reply.
  pkt->typeIs(ICMPPacket::kEchoReply);

  // Recompute checksums.
  pkt->checksumReset();
  ip_pkt->checksumReset();

  // Send the Echo Reply packet.
  outputPacketNew(ip_pkt);
}


void ControlPlane::sendICMPTTLExceeded(IPPacket::Ptr orig_pkt) {
  DLOG << "sending ICMP TTL Exceeded message to " << orig_pkt->src();

  // Send at most IP header + 8 bytes of data.
  const size_t max_data_len = orig_pkt->headerLen() + 8;
  const size_t data_len =
      (orig_pkt->len() < max_data_len) ? orig_pkt->len() : max_data_len;

  // Create buffer for new packet.
  const size_t pkt_len = (IPPacket::kHeaderSize +
                          ICMPPacket::kHeaderLen +
                          data_len);
  PacketBuffer::Ptr buffer = PacketBuffer::New(pkt_len);

  // Look up routing table entry for packet source.
  IPv4Addr dest_ip = orig_pkt->src();
  RoutingTable::Entry::Ptr r_entry;
  {
    RoutingTable::Ptr rtable = dp_->controlPlane()->routingTable();
    Fwk::ScopedLock<RoutingTable> lock(rtable);
    r_entry = rtable->lpm(dest_ip);
  }
  if (!r_entry) {
    DLOG << "No route to " << dest_ip << ", giving up.";
    return;
  }

  // Outgoing interface.
  Interface::PtrConst out_iface = r_entry->interface();

  // IP packet next.
  IPPacket::Ptr ip_pkt = IPPacket::New(buffer, buffer->size() - pkt_len);
  ip_pkt->versionIs(4);
  ip_pkt->headerLengthIs(IPPacket::kHeaderSize / 4);  // words, not bytes!
  ip_pkt->packetLengthIs(pkt_len);
  ip_pkt->diffServicesAre(0);
  ip_pkt->protocolIs(IPPacket::kICMP);
  ip_pkt->identificationIs(0);
  ip_pkt->flagsAre(IPPacket::kIP_DF);
  ip_pkt->fragmentOffsetIs(0);
  ip_pkt->srcIs(out_iface->ip());
  ip_pkt->dstIs(orig_pkt->src());
  ip_pkt->ttlIs(IPPacket::kDefaultTTL);

  // ICMP subtype packet.
  // Cannot use a direct static cast here because we need to call the subtype
  // constructor.
  ICMPPacket::Ptr icmp_pkt = Ptr::st_cast<ICMPPacket>(ip_pkt->payload());
  ICMPTimeExceededPacket::Ptr icmp_te_pkt =
      ICMPTimeExceededPacket::New(icmp_pkt);
  icmp_te_pkt->codeIs(ICMPPacket::kTTLExceeded);
  icmp_te_pkt->originalPacketIs(orig_pkt);

  // Recompute checksums in reverse order.
  icmp_te_pkt->checksumReset();
  ip_pkt->checksumReset();

  // Send packet.
  outputPacketNew(ip_pkt);
}


void ControlPlane::sendICMPDestNetworkUnreach(IPPacket::PtrConst orig_pkt) {
  DLOG << "sending ICMP Destination Network Unreachable to " << orig_pkt->src();
  sendICMPDestUnreach(ICMPPacket::kNetworkUnreach, orig_pkt);
}


void ControlPlane::sendICMPDestHostUnreach(IPPacket::PtrConst orig_pkt) {
  DLOG << "sending ICMP Destination Host Unreachable to " << orig_pkt->src();
  sendICMPDestUnreach(ICMPPacket::kHostUnreach, orig_pkt);
}


void ControlPlane::sendICMPDestProtoUnreach(IPPacket::PtrConst orig_pkt) {
  DLOG << "sending ICMP Destination Proto Unreachable to " << orig_pkt->src();
  sendICMPDestUnreach(ICMPPacket::kProtoUnreach, orig_pkt);
}


void
ControlPlane::sendICMPDestUnreachFragRequired(IPPacket::PtrConst orig_pkt) {
  DLOG << "sending ICMP Fragmentation Required to " << orig_pkt->src();
  sendICMPDestUnreach(ICMPPacket::kFragRequired, orig_pkt);
}


void ControlPlane::sendICMPDestUnreach(const ICMPPacket::Code code,
                                       IPPacket::PtrConst orig_pkt) {
  // Send at most IP header + 8 bytes of data.
  const size_t max_data_len = orig_pkt->headerLen() + 8;
  const size_t data_len =
      (orig_pkt->len() < max_data_len) ? orig_pkt->len() : max_data_len;

  // Create buffer for new packet.
  const size_t pkt_len = (IPPacket::kHeaderSize +
                          ICMPPacket::kHeaderLen +
                          data_len);
  PacketBuffer::Ptr buffer = PacketBuffer::New(pkt_len);

  // Look up routing table entry for packet source.
  IPv4Addr dest_ip = orig_pkt->src();
  RoutingTable::Entry::Ptr r_entry;
  {
    RoutingTable::Ptr rtable = dp_->controlPlane()->routingTable();
    Fwk::ScopedLock<RoutingTable> lock(rtable);
    r_entry = rtable->lpm(dest_ip);
  }
  if (!r_entry) {
    DLOG << "No route to " << dest_ip << ", giving up.";
    return;
  }

  // Outgoing interface.
  Interface::PtrConst out_iface = r_entry->interface();

  // IP packet next.
  IPPacket::Ptr ip_pkt = IPPacket::New(buffer, buffer->size() - pkt_len);
  ip_pkt->versionIs(4);
  ip_pkt->headerLengthIs(IPPacket::kHeaderSize / 4);  // words, not bytes!
  ip_pkt->packetLengthIs(pkt_len);
  ip_pkt->diffServicesAre(0);
  ip_pkt->protocolIs(IPPacket::kICMP);
  ip_pkt->identificationIs(0);
  ip_pkt->flagsAre(IPPacket::kIP_DF);
  ip_pkt->fragmentOffsetIs(0);
  ip_pkt->srcIs(out_iface->ip());
  ip_pkt->dstIs(orig_pkt->src());
  ip_pkt->ttlIs(IPPacket::kDefaultTTL);

  // ICMP subtype packet.
  ICMPPacket::Ptr icmp_pkt = Ptr::st_cast<ICMPPacket>(ip_pkt->payload());
  ICMPDestUnreachablePacket::Ptr icmp_du_pkt =
      ICMPDestUnreachablePacket::New(icmp_pkt);
  icmp_du_pkt->codeIs(code);
  icmp_du_pkt->originalPacketIs(orig_pkt);

  // Recompute checksums in reverse order.
  icmp_du_pkt->checksumReset();
  ip_pkt->checksumReset();

  // Send packet.
  outputPacketNew(ip_pkt);
}


void ControlPlane::encapsulateAndOutputPacket(IPPacket::Ptr pkt,
                                              Interface::PtrConst out_iface) {
  DLOG << "Encapsulating packet for virtual interface";

  // Look up the associated tunnel.
  Tunnel::Ptr tunnel;
  {
    Fwk::ScopedLock<TunnelMap> tunnel_map_lock(tunnel_map_);
    tunnel = tunnel_map_->tunnel(out_iface->name());
    if (!tunnel) {
      ELOG << "output interface is virtual but no associated tunnel found";
      return;
    }
  }

  DLOG << "  tunnel remote: " << tunnel->remote();

  if (tunnel->mode() != Tunnel::kGRE) {
    ELOG << "unknown tunnel mode; cannot encapsulate";
    return;
  }

  // Do an LPM on remote.
  RoutingTable::Entry::Ptr tunnel_r_entry;
  {
    RoutingTable::Ptr rtable = dp_->controlPlane()->routingTable();
    Fwk::ScopedLock<RoutingTable> lock(rtable);
    tunnel_r_entry = rtable->lpm(tunnel->remote());
  }
  if (!tunnel_r_entry) {
    // TODO(ms): is there something we can do besides give up here?
    DLOG << "No route to " << tunnel->remote() << ", giving up";
    return;
  }

  // Add GRE header.
  // TODO(ms): kHeaderSize assumes we will have a checksum!
  pkt->buffer()->minimumSizeIs(pkt->len() + GREPacket::kHeaderSize);
  GREPacket::Ptr gre_pkt =
      GREPacket::GREPacketNew(pkt->buffer(),
                              pkt->bufferOffset() - GREPacket::kHeaderSize);
  gre_pkt->checksumPresentIs(true);
  gre_pkt->reserved0Is(0);
  gre_pkt->reserved1Is(0);
  gre_pkt->versionIs(0);
  gre_pkt->protocolIs(EthernetPacket::kIP);
  gre_pkt->checksumReset();

  // Add IP header.
  gre_pkt->buffer()->minimumSizeIs(gre_pkt->len() + IPPacket::kHeaderSize);
  IPPacket::Ptr ip_pkt =
      IPPacket::New(gre_pkt->buffer(),
                    gre_pkt->bufferOffset() - IPPacket::kHeaderSize);
  ip_pkt->versionIs(4);
  ip_pkt->headerLengthIs(IPPacket::kHeaderSize / 4);  // words, not bytes!
  ip_pkt->packetLengthIs(ip_pkt->len());
  ip_pkt->diffServicesAre(0);
  ip_pkt->protocolIs(IPPacket::kGRE);
  ip_pkt->identificationIs(0);
  ip_pkt->flagsAre(0);
  ip_pkt->fragmentOffsetIs(0);
  ip_pkt->srcIs(tunnel_r_entry->interface()->ip());
  ip_pkt->dstIs(tunnel->remote());
  ip_pkt->ttlIs(IPPacket::kDefaultTTL);
  ip_pkt->checksumReset();

  // Output new packet.
  outputPacketNew(ip_pkt);
}


void ControlPlane::fragmentAndSend(IPPacket::Ptr pkt) {
  if (pkt->flags() & IPPacket::kIP_DF) {
    WLOG << "Tried to fragment a packet with Don't Fragment bit set; ignoring";
    return;
  }

  // Must be a multiple of 8.
  const size_t max_payload_size = 1480;
  const uint16_t cksum = pkt->checksum();

  size_t bytes_left = pkt->len() - IPPacket::kHeaderSize;
  size_t fragment_offset = 0;  // in 8-byte blocks
  while (bytes_left > 0) {
    // Calculate size of this fragment's payload.
    size_t fragment_payload_size;
    bool last_fragment;
    if (bytes_left > max_payload_size) {
      fragment_payload_size = max_payload_size;
      last_fragment = false;
    } else {
      fragment_payload_size = bytes_left;
      last_fragment = true;
    }

    // Total size of fragment.
    const size_t fragment_size = IPPacket::kHeaderSize + fragment_payload_size;

    // Create a new buffer and IP packet for new fragment.
    PacketBuffer::Ptr buffer = PacketBuffer::New(fragment_size);
    IPPacket::Ptr fragment =
        IPPacket::New(buffer, buffer->size() - fragment_size);

    // Copy fields into fragment.
    fragment->versionIs(4);
    fragment->headerLengthIs(IPPacket::kHeaderSize / 4);  // words
    fragment->packetLengthIs(fragment->len());
    fragment->diffServicesAre(pkt->diffServices());
    fragment->protocolIs(pkt->protocol());
    fragment->srcIs(pkt->src());
    fragment->dstIs(pkt->dst());
    fragment->ttlIs(pkt->ttl());

    // Set fragmentation fields.
    fragment->identificationIs(cksum);  // use cksum as identifier
    fragment->flagsAre(last_fragment ? 0 : IPPacket::kIP_MF);
    fragment->fragmentOffsetIs(fragment_offset);

    // Copy portion of data.
    memcpy(fragment->data() + IPPacket::kHeaderSize,
           pkt->data() + IPPacket::kHeaderSize + (fragment_offset * 8),
           fragment_payload_size);

    // Update checksum.
    fragment->checksumReset();

    // Send fragment.
    outputPacketNew(fragment);

    bytes_left -= fragment_payload_size;
    fragment_offset += fragment_payload_size / 8;
  }
}
