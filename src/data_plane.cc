#include "data_plane.h"

#include <string>
#include "fwk/log.h"
#include "fwk/named_interface.h"

#include "arp_packet.h"
#include "ethernet_packet.h"
#include "icmp_packet.h"
#include "ip_packet.h"

using std::string;


DataPlane::DataPlane(const std::string& name)
    : Fwk::NamedInterface(name), log_(Fwk::Log::LogNew(name)), functor_(this) {
  
}


void DataPlane::packetNew(EthernetPacket::Ptr pkt) {
  (*pkt)(&functor_);
}

DataPlane::PacketFunctor::PacketFunctor(DataPlane* const dp)
    : dp_(dp), log_(dp->log_) { }

void DataPlane::PacketFunctor::operator()(ARPPacket* const pkt) {

}

void DataPlane::PacketFunctor::operator()(EthernetPacket* const pkt) {
  (*log_)() << "EthernetPacket dispatch in DataPlane";
  (*log_)() << "  src: " << pkt->src();
  (*log_)() << "  dst: " << pkt->dst();
  (*log_)() << "  type: " << pkt->typeName();
}

void DataPlane::PacketFunctor::operator()(ICMPPacket* const pkt) {

}

void DataPlane::PacketFunctor::operator()(IPPacket* const pkt) {

}

void DataPlane::PacketFunctor::operator()(UnknownPacket* const pkt) {

}
