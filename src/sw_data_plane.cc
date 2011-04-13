#include "sw_data_plane.h"

#include "fwk/log.h"
#include "fwk/utility.h"

#include "ethernet_packet.h"
#include "sr_integration.h"

struct sr_instance;


SWDataPlane::SWDataPlane(struct sr_instance *sr,
                         RoutingTable::Ptr routing_table,
                         ARPCache::Ptr arp_cache)
    : DataPlane("SWDataPlane", sr, routing_table, arp_cache) {
  log_ = Fwk::Log::LogNew("SWDataPlane");
  log_->entryNew("constructor");
}


void SWDataPlane::outputPacketNew(EthernetPacket::Ptr pkt,
                                  Interface::PtrConst iface) {
  DLOG << "outputPacketNew() in SWDataPlane";
  DLOG << "  iface: " << iface->name();
  DLOG << "  src: " << pkt->src();
  DLOG << "  dst: " << pkt->dst();
  DLOG << "  type: " << pkt->typeName();
  DLOG << "  length: " << pkt->len();

  debug_dump64((const void *)pkt->data(), pkt->len());

  struct sr_instance* sr = instance();
  sr_integ_low_level_output(sr, pkt->data(), pkt->len(), iface->name().c_str());
}
