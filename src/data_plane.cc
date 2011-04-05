#include "data_plane.h"

#include <string>
#include "fwk/log.h"
#include "fwk/named_interface.h"

#include "arp_packet.h"
#include "ethernet_packet.h"
#include "icmp_packet.h"
#include "ip_packet.h"


DataPlane::DataPlane(const std::string& name) : Fwk::NamedInterface(name) {
  log_ = Fwk::Log::LogNew(name);
  functor_ = new PacketFunctor(this);
}

DataPlane::~DataPlane() {
  delete functor_;
}

void DataPlane::packetNew(EthernetPacket::Ptr pkt) {
  (*pkt)(functor_);
}

DataPlane::PacketFunctor::PacketFunctor(DataPlane* const dp)
    : dp_(dp), log_(dp->log_) { }

void DataPlane::PacketFunctor::operator()(ARPPacket* const pkt) {
  (*log_)() << "ARPPacket dispatch in DataPlane";
}

void DataPlane::PacketFunctor::operator()(EthernetPacket* const pkt) {
  (*log_)() << "EthernetPacket dispatch in DataPlane";
  (*log_)() << "  src: " << pkt->src();
  (*log_)() << "  dst: " << pkt->dst();
  (*log_)() << "  type: " << pkt->typeName();

  // Dispatch encapsulated packet.
  Packet::Ptr payload_pkt = pkt->payload();
  (*payload_pkt)(this);
}

void DataPlane::PacketFunctor::operator()(ICMPPacket* const pkt) {

}

void DataPlane::PacketFunctor::operator()(IPPacket* const pkt) {
  (*log_)() << "IPPacket dispatch in DataPlane";
}
