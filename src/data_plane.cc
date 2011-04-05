#include "data_plane.h"

#include <string>
#include "fwk/log.h"
#include "fwk/named_interface.h"

#include "arp_packet.h"
#include "ethernet_packet.h"
#include "icmp_packet.h"
#include "ip_packet.h"

using std::string;


DataPlane::DataPlane(const std::string& name) : Fwk::NamedInterface(name) {
  functor_ = new PacketFunctor(this);
  log_ = Fwk::Log::LogNew(name);
}

DataPlane::~DataPlane() {
  delete functor_;
}

void DataPlane::packetNew(EthernetPacket::Ptr pkt) {
  (*pkt)(functor_);
}

DataPlane::PacketFunctor::PacketFunctor(DataPlane* const dp) : dp_(dp) { }

void DataPlane::PacketFunctor::operator()(ARPPacket* const pkt) {

}

void DataPlane::PacketFunctor::operator()(EthernetPacket* const pkt) {
  dp_->log_->entryNew("EthernetPacket dispatch in DataPlane");
  (*(dp_->log_))() << "  src: " << pkt->src();
  (*(dp_->log_))() << "  dst: " << pkt->dst();
}

void DataPlane::PacketFunctor::operator()(ICMPPacket* const pkt) {

}

void DataPlane::PacketFunctor::operator()(IPPacket* const pkt) {

}
