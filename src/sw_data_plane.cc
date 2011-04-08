#include "sw_data_plane.h"

#include "ethernet_packet.h"
#include "fwk/log.h"
#include "sr_integration.h"

struct sr_instance;


SWDataPlane::SWDataPlane() : DataPlane("SWDataPlane") {
  log_ = Fwk::Log::LogNew("SWDataPlane");

  log_->entryNew("constructor");
}


void SWDataPlane::outputPacketNew(EthernetPacket::Ptr pkt,
                                  Interface::PtrConst iface) {
  DLOG << "outputPacketNew() in SWDataPlane";
  DLOG << "  iface: " << iface->name();
  DLOG << "  src: " << pkt->src();
  DLOG << "  dst: " << pkt->src();
  DLOG << "  type: " << pkt->typeName();
  DLOG << "  length: " << pkt->len();

  struct sr_instance* sr = instance();
  sr_integ_low_level_output(sr, pkt->data(), pkt->len(), iface->name().c_str());
}
