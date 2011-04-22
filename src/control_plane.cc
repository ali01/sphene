#include "control_plane.h"

#include <string>
#include "fwk/buffer.h"
#include "fwk/log.h"
#include "fwk/named_interface.h"

#include "arp_packet.h"
#include "ethernet_packet.h"
#include "icmp_packet.h"
#include "interface.h"
#include "interface_map.h"
#include "ip_packet.h"

// Input to TCP stack.
void sr_transport_input(uint8_t* packet);


ControlPlane::ControlPlane(const std::string& name)
    : Fwk::NamedInterface(name),
      log_(Fwk::Log::LogNew(name)),
      functor_(this),
      arp_cache_(ARPCache::New()),
      arp_queue_(ARPQueue::New()),
      routing_table_(RoutingTable::New()) { }


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

  // Look up routing table entry of the longest prefix match.
  IPv4Addr dest_ip = pkt->dst();
  RoutingTable::Entry::Ptr r_entry;
  {
    RoutingTable::Ptr rtable = dp_->controlPlane()->routingTable();
    RoutingTable::ScopedLock lock(rtable);
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
  Interface::Ptr out_iface = r_entry->interface();

  // Next hop IP address.
  IPv4Addr next_hop_ip = r_entry->gateway();

  // Look up ARP entry for next hop.
  ARPCache::Entry::Ptr arp_entry;
  {
    ARPCache::Ptr cache = dp_->controlPlane()->arpCache();
    ARPCache::ScopedLock lock(cache);
    arp_entry = cache->entry(next_hop_ip);
  }
  if (!arp_entry) {
    DLOG << "ARP Cache miss for " << string(next_hop_ip);
    sendARPRequestAndEnqueuePacket(next_hop_ip, out_iface, pkt);
    return;
  }

  // Decrement TTL. We do this last to avoid having to increase the TTL and
  // recompute the checksum again if an error occurs.
  pkt->ttlDec(1);
  DLOG << "  decremented TTL: " << (uint32_t)pkt->ttl();
  if (pkt->ttl() < 1) {
    // Send ICMP Time Exceeded Message to source.
    pkt->ttlIs(pkt->ttl() + 1);
    sendICMPTTLExceeded(pkt);
    return;
  }

  // Recompute the checksum since we changed the TTL.
  pkt->checksumReset();

  // Update Ethernet header using ARP entry.
  EthernetPacket::Ptr eth_pkt =
      Ptr::st_cast<EthernetPacket>(pkt->enclosingPacket());
  eth_pkt->srcIs(out_iface->mac());
  eth_pkt->dstIs(arp_entry->ethernetAddr());

  // Send packet.
  DLOG << "Forwarding IP packet to " << string(next_hop_ip);
  dp_->outputPacketNew(eth_pkt, out_iface);
}


ControlPlane::PacketFunctor::PacketFunctor(ControlPlane* const cp)
    : cp_(cp), log_(cp->log_) { }


void ControlPlane::PacketFunctor::operator()(ARPPacket* const pkt,
                                             const Interface::PtrConst iface) {
  // TODO(ms): Validate all of the fields in the ARP packet.

  DLOG << "ARPPacket dispatch in ControlPlane";
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
  cp_->arpCache()->lockedIs(true);
  ARPCache::Entry::Ptr cache_entry = cp_->arpCache()->entry(sender_ip);
  cp_->arpCache()->lockedIs(false);
  if (cache_entry) {
    cache_entry->ethernetAddrIs(sender_eth);
    cache_entry->ageIs(0);
    merge_flag = true;
  }

  // Are we the target of the ARP packet?
  if (cp_->dataPlane()->interfaceMap()->interfaceAddr(target_ip)) {
    DLOG << "We are the target of this ARP packet.";

    // Add <sender IP, sender MAC> to ARP cache.
    if (!merge_flag) {
      cache_entry = ARPCache::Entry::New(sender_ip, sender_eth);
      cache_entry->typeIs(ARPCache::Entry::kDynamic);
      cp_->arpCache()->lockedIs(true);
      cp_->arpCache()->entryIs(cache_entry);
      cp_->arpCache()->lockedIs(false);
    }

    // Look at the opcode.
    if (pkt->operation() == ARPPacket::kRequest) {
      DLOG << "sending ARP reply";

      // Swap the hardware and protocol fields.
      pkt->targetHWAddrIs(sender_eth);
      pkt->targetPAddrIs(sender_ip);
      pkt->senderHWAddrIs(target_eth);
      pkt->senderPAddrIs(target_ip);

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

      DLOG << "ARP reply for " << ip_addr << " received.";

      cp_->cacheMapping(ip_addr, eth_addr);
      cp_->sendEnqueued(ip_addr, eth_addr);
    }
  }
}


void ControlPlane::PacketFunctor::operator()(EthernetPacket* const pkt,
                                             const Interface::PtrConst iface) {
  DLOG << "EthernetPacket dispatch in ControlPlane";

  // Dispatch encapsulated packet.
  Packet::Ptr payload_pkt = pkt->payload();
  (*payload_pkt)(this, iface);
}


void ControlPlane::PacketFunctor::operator()(ICMPPacket* const pkt,
                                             const Interface::PtrConst iface) {
  // TODO(ms): Further dispatch of ICMPPacket types should probably be here.
  DLOG << "ICMPPacket dispatch in ControlPlane";
  DLOG << "  type: " << pkt->type() << " (" << pkt->typeName() << ")";
  DLOG << "  code: " << (uint32_t)pkt->code();

  IPPacket::Ptr ip_pkt = (IPPacket*)(pkt->enclosingPacket().ptr());

  // Handle ICMP Echo Requests.
  if (pkt->type() == ICMPPacket::kEchoRequest) {
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
    cp_->outputPacketNew(ip_pkt);
  }
}


void ControlPlane::PacketFunctor::operator()(IPPacket* const pkt,
                                             const Interface::PtrConst iface) {
  DLOG << "IPPacket dispatch in ControlPlane";

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


void ControlPlane::PacketFunctor::operator()(UnknownPacket* const pkt,
                                             const Interface::PtrConst iface) {

}


void
ControlPlane::sendARPRequestAndEnqueuePacket(IPv4Addr next_hop_ip,
                                             Interface::Ptr out_iface,
                                             IPPacket::Ptr pkt) {
  // Do we already have a packet queue for this next hop in the arp queue?
  ARPQueue::Entry::Ptr entry = arp_queue_->entry(next_hop_ip);
  if (entry == NULL) {
    // No entry in the ARP queue yet.
    size_t pkt_len = EthernetPacket::kHeaderSize + ARPPacket::kHeaderSize;

    Fwk::Buffer::Ptr buffer = Fwk::Buffer::BufferNew(pkt_len);
    EthernetPacket::Ptr req_eth_pkt = EthernetPacket::New(buffer, 0);

    req_eth_pkt->srcIs(out_iface->mac());
    req_eth_pkt->dstIs(EthernetAddr::kBroadcast);
    req_eth_pkt->typeIs(EthernetPacket::kARP);

    ARPPacket::Ptr arp_pkt = Ptr::st_cast<ARPPacket>(req_eth_pkt->payload());
    arp_pkt->operationIs(ARPPacket::kRequest);
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
  EthernetPacket::Ptr eth_pkt =
      Ptr::st_cast<EthernetPacket>(pkt->enclosingPacket());
  entry->packetIs(eth_pkt);
}


void
ControlPlane::cacheMapping(IPv4Addr ip_addr, EthernetAddr eth_addr) {
  // Adding mapping to ARP cache.
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
      eth_pkt->dstIs(eth_addr);

      DLOG << "  forwarding queued packet to " << ip_addr;
      dp_->outputPacketNew(eth_pkt, out_iface);

      pkt_wrapper = pkt_wrapper->next();
    }

    arp_queue_->entryDel(queue_entry);
  }
}


void ControlPlane::sendICMPTTLExceeded(IPPacket::Ptr orig_pkt) {
  DLOG << "sending ICMP TTL Exceeded message to " << orig_pkt->src();

  // Send at most IP header + 8 bytes of data.
  const size_t max_data_len = orig_pkt->headerLen() + 8;
  const size_t data_len =
      (orig_pkt->len() < max_data_len) ? orig_pkt->len() : max_data_len;

  // Create buffer for new packet.
  const size_t pkt_len = (EthernetPacket::kHeaderSize +
                          IPPacket::kHeaderSize +
                          ICMPPacket::kHeaderLen +
                          data_len);
  Fwk::Buffer::Ptr buffer = Fwk::Buffer::BufferNew(pkt_len);

  // Look up routing table entry for packet source.
  IPv4Addr dest_ip = orig_pkt->src();
  RoutingTable::Entry::Ptr r_entry;
  {
    RoutingTable::Ptr rtable = dp_->controlPlane()->routingTable();
    RoutingTable::ScopedLock lock(rtable);
    r_entry = rtable->lpm(dest_ip);
  }
  if (!r_entry) {
    DLOG << "No route to " << dest_ip << ", giving up.";
    return;
  }

  // Outgoing interface.
  Interface::Ptr out_iface = r_entry->interface();

  // Ethernet packet first. Src and Dst are set when the IP packet is sent.
  EthernetPacket::Ptr eth_pkt = EthernetPacket::New(buffer, 0);
  eth_pkt->typeIs(EthernetPacket::kIP);

  // IP packet next.
  IPPacket::Ptr ip_pkt =
      Ptr::st_cast<IPPacket>(eth_pkt->payload());
  ip_pkt->versionIs(4);
  ip_pkt->headerLengthIs(IPPacket::kHeaderSize / 4);  // words, not bytes!
  ip_pkt->packetLengthIs(pkt_len - EthernetPacket::kHeaderSize);
  ip_pkt->diffServicesAre(0);
  ip_pkt->protocolIs(IPPacket::kICMP);
  ip_pkt->flagsAre(IPPacket::IP_DF);
  ip_pkt->fragmentOffsetIs(0);
  ip_pkt->srcIs(out_iface->ip());
  ip_pkt->dstIs(orig_pkt->src());
  // TODO(ms): Any reason to pick a better default?
  ip_pkt->ttlIs(64);

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


void ControlPlane::sendICMPDestHostUnreach(IPPacket::PtrConst orig_pkt) {
  DLOG << "sending ICMP Destination Host Unreachable to " << orig_pkt->src();
  sendICMPDestUnreach(ICMPPacket::kHostUnreach, orig_pkt);
}


void ControlPlane::sendICMPDestProtoUnreach(IPPacket::PtrConst orig_pkt) {
  DLOG << "sending ICMP Destination Proto Unreachable to " << orig_pkt->src();
  sendICMPDestUnreach(ICMPPacket::kProtoUnreach, orig_pkt);
}


void ControlPlane::sendICMPDestUnreach(const ICMPPacket::Code code,
                                       IPPacket::PtrConst orig_pkt) {
  // Send at most IP header + 8 bytes of data.
  const size_t max_data_len = orig_pkt->headerLen() + 8;
  const size_t data_len =
      (orig_pkt->len() < max_data_len) ? orig_pkt->len() : max_data_len;

  // Create buffer for new packet.
  const size_t pkt_len = (EthernetPacket::kHeaderSize +
                          IPPacket::kHeaderSize +
                          ICMPPacket::kHeaderLen +
                          data_len);
  Fwk::Buffer::Ptr buffer = Fwk::Buffer::BufferNew(pkt_len);

  // Look up routing table entry for packet source.
  IPv4Addr dest_ip = orig_pkt->src();
  RoutingTable::Entry::Ptr r_entry;
  {
    RoutingTable::Ptr rtable = dp_->controlPlane()->routingTable();
    RoutingTable::ScopedLock lock(rtable);
    r_entry = rtable->lpm(dest_ip);
  }
  if (!r_entry) {
    DLOG << "No route to " << dest_ip << ", giving up.";
    return;
  }

  // Outgoing interface.
  Interface::Ptr out_iface = r_entry->interface();

  // Ethernet packet first. Src and Dst are set when the IP packet is sent.
  EthernetPacket::Ptr eth_pkt = EthernetPacket::New(buffer, 0);
  eth_pkt->typeIs(EthernetPacket::kIP);

  // IP packet next.
  IPPacket::Ptr ip_pkt =
      Ptr::st_cast<IPPacket>(eth_pkt->payload());
  ip_pkt->versionIs(4);
  ip_pkt->headerLengthIs(IPPacket::kHeaderSize / 4);  // words, not bytes!
  ip_pkt->packetLengthIs(pkt_len - EthernetPacket::kHeaderSize);
  ip_pkt->diffServicesAre(0);
  ip_pkt->protocolIs(IPPacket::kICMP);
  ip_pkt->flagsAre(IPPacket::IP_DF);
  ip_pkt->fragmentOffsetIs(0);
  ip_pkt->srcIs(out_iface->ip());
  ip_pkt->dstIs(orig_pkt->src());
  ip_pkt->ttlIs(64);

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
