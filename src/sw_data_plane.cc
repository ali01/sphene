#include "sw_data_plane.h"

#include "fwk/log.h"

#include "ethernet_packet.h"
#include "ip_packet.h"
#include "sr_integration.h"

struct sr_instance;


SWDataPlane::SWDataPlane(struct sr_instance* sr,
                         RoutingTable::Ptr routing_table,
                         ARPCache::Ptr arp_cache)
    : DataPlane("SWDataPlane", sr, routing_table, arp_cache),
      log_(Fwk::Log::LogNew("SWDataPlane")) { }


void SWDataPlane::outputPacketNew(EthernetPacket::PtrConst pkt,
                                  Interface::PtrConst iface) {
  DLOG << "outputPacketNew() in SWDataPlane";
  DLOG << "  iface: " << iface->name();
  DLOG << "  src: " << pkt->src();
  DLOG << "  dst: " << pkt->dst();
  DLOG << "  length: " << pkt->len();
  DLOG << "  type: " << pkt->typeName();

  if (pkt->type() == EthernetPacket::kIP) {
    EthernetPacket* _pkt = const_cast<EthernetPacket*>(pkt.ptr());
    IPPacket::Ptr ip_pkt = Ptr::st_cast<IPPacket>(_pkt->payload());
    DLOG << "    src: " << ip_pkt->src();
    DLOG << "    dst: " << ip_pkt->dst();
    DLOG << "    identification: " << ip_pkt->identification();
    DLOG << "    flags: " << (uint32_t)ip_pkt->flags();
    DLOG << "    fragment offset: " << ip_pkt->fragmentOffset();
    DLOG << "    ttl: " << (uint32_t)ip_pkt->ttl();
    DLOG << "    protocol: " << ip_pkt->protocol();
    DLOG << "    header checksum: " << ip_pkt->checksum();
    DLOG << "    length: " << ip_pkt->packetLength();
  }

  struct sr_instance* sr = instance();
  sr_integ_low_level_output(sr, pkt->data(), pkt->len(), iface->name().c_str());
}
