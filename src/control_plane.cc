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


// TODO(ms): This function probably shouldn't need an iface
//   argument. Theoretically, outgoing packets should not be destined for the
//   router, and if they are, that's probably an error.
void ControlPlane::outputPacketNew(IPPacket::Ptr pkt,
                                   Interface::PtrConst iface) {
  if (pkt->buffer()->len() >= 5 && pkt->headerLength() >= 5 &&
      pkt->version() == 4 && pkt->checksumValid()) {

    // IP Packet's destination
    IPv4Addr dest_ip = pkt->dst();
    Interface::Ptr target_iface = dp_->interfaceMap()->interfaceAddr(dest_ip);

    if (target_iface == NULL) { // Packet is not destined to router
      // Decrementing TTL
      pkt->ttlDec(1);

      if (pkt->ttl() > 0) {
        // Recomputing checksum
        pkt->checksumReset();

        // Finding longest prefix match
        RoutingTable::Entry::Ptr r_entry = routing_table_->lpm(dest_ip);
        if (r_entry) {
          Interface::Ptr out_iface = r_entry->interface();

          IPv4Addr next_hop_ip = r_entry->gateway();
          ARPCache::Entry::Ptr arp_entry = arp_cache_->entry(next_hop_ip);

          if (arp_entry) {
            EthernetPacket::Ptr eth_pkt =
              Ptr::st_cast<EthernetPacket>(pkt->enclosingPacket());

            eth_pkt->srcIs(out_iface->mac());
            eth_pkt->dstIs(arp_entry->ethernetAddr());

            // Forwarding packet.
            DLOG << "Forwarding IP packet to " << string(next_hop_ip);
            dp_->outputPacketNew(eth_pkt, out_iface);

          } else {
            DLOG << "ARP Cache miss for " << string(next_hop_ip);
            sendARPRequest(next_hop_ip, out_iface);
            enqueuePacket(next_hop_ip, out_iface, pkt);
          }

        } else {
          DLOG << "Route for " << string(dest_ip)
               << " does not exist in RoutingTable.";
          // TODO: send ICMP no route to host.
        }

      } else {
        // Send ICMP Time Exceeded Message to source
        // TODO
      }

    } else {
      // Packet is destined to router; dispatch to control plane functor.
      this->packetNew(pkt, iface);
    }
  }
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
    cp_->outputPacketNew(ip_pkt, NULL);
  }
}


void ControlPlane::PacketFunctor::operator()(IPPacket* const pkt,
                                             const Interface::PtrConst iface) {
  DLOG << "IPPacket dispatch in ControlPlane";

  // Dispatch encapsulated packet.
  Packet::Ptr payload_pkt = pkt->payload();
  (*payload_pkt)(this, iface);
}


void ControlPlane::PacketFunctor::operator()(UnknownPacket* const pkt,
                                             const Interface::PtrConst iface) {

}

void
ControlPlane::sendARPRequest(IPv4Addr ip_addr, Interface::Ptr out_iface) {
  size_t pkt_len = EthernetPacket::kHeaderSize + ARPPacket::kHeaderSize;

  Fwk::Buffer::Ptr buffer = Fwk::Buffer::BufferNew(pkt_len);
  EthernetPacket::Ptr eth_pkt = EthernetPacket::New(buffer, 0);

  eth_pkt->srcIs(out_iface->mac());
  eth_pkt->dstIs(EthernetAddr::kBroadcast);
  eth_pkt->typeIs(EthernetPacket::kARP);

  ARPPacket::Ptr arp_pkt = Ptr::st_cast<ARPPacket>(eth_pkt->payload());
  arp_pkt->operationIs(ARPPacket::kRequest);
  arp_pkt->senderHWAddrIs(out_iface->mac());
  arp_pkt->senderPAddrIs(out_iface->ip());
  arp_pkt->targetHWAddrIs(EthernetAddr::kZero);
  arp_pkt->targetPAddrIs(ip_addr);

  DLOG << "Sending ARP request for " << string(ip_addr);
  DLOG << "  ARP sender HW addr: " << arp_pkt->senderHWAddr();
  DLOG << "  ARP sender P addr:  " << arp_pkt->senderPAddr();
  DLOG << "  ARP target HW addr: " << arp_pkt->targetHWAddr();
  DLOG << "  ARP target P addr:  " << arp_pkt->targetPAddr();

  dp_->outputPacketNew(eth_pkt, out_iface);
}

void
ControlPlane::enqueuePacket(IPv4Addr next_hop_ip, Interface::Ptr out_iface,
                            IPPacket::Ptr pkt) {
  // Adding packet to ARP queue.
  ARPQueue::Entry::Ptr entry = arp_queue_->entry(next_hop_ip);
  if (entry == NULL) {
    entry = ARPQueue::Entry::New(next_hop_ip, out_iface);
    arp_queue_->entryIs(entry);
  }

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
  ARPQueue::Entry::Ptr queue_entry = arp_queue_->entry(ip_addr);
  if (queue_entry) {
    EthernetPacket::Ptr pkt;
    Interface::Ptr out_iface = queue_entry->interface();
    ARPQueue::PacketWrapper::Ptr pkt_wrapper = queue_entry->front();
    while (pkt_wrapper != NULL) {
      pkt = pkt_wrapper->packet();
      pkt->dstIs(eth_addr);
      
      DLOG << "Forwarding queued packet to " << ip_addr;
      dp_->outputPacketNew(pkt, out_iface);

      pkt_wrapper = pkt_wrapper->next();
    }

    arp_queue_->entryDel(queue_entry);
  }
}
