#include "sw_data_plane.h"

#include "ethernet_packet.h"
#include "fwk/log.h"


SWDataPlane::SWDataPlane() : DataPlane("SWDataPlane") {
  log_ = Fwk::Log::LogNew("SWDataPlane");

  log_->entryNew("constructor");
}


void SWDataPlane::outputPacketNew(EthernetPacket::Ptr pkt,
                                  Interface::PtrConst iface) {
  DLOG << "outputPacketNew() in SWDataPlane";
}
