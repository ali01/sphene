#include "data_plane.h"

#include <string>
#include "fwk/log.h"
#include "fwk/named_interface.h"

#include "arp_packet.h"
#include "ethernet_packet.h"
#include "icmp_packet.h"
#include "interface.h"
#include "ip_packet.h"


DataPlane::DataPlane(const std::string& name)
    : Fwk::NamedInterface(name),
      log_(Fwk::Log::LogNew(name)),
      functor_(this),
      iface_map_(InterfaceMap::InterfaceMapNew()) { }


void DataPlane::packetNew(EthernetPacket::Ptr pkt,
                          const Interface::PtrConst iface) {
  // Dispatch packet using double-dispatch.
  (*pkt)(&functor_, iface);
}


InterfaceMap::Ptr DataPlane::interfaceMap() const {
  return iface_map_;
}


DataPlane::PacketFunctor::PacketFunctor(DataPlane* const dp)
    : dp_(dp), log_(dp->log_) { }


void DataPlane::PacketFunctor::operator()(ARPPacket* const pkt,
                                          const Interface::PtrConst iface) {
  DLOG << "ARPPacket dispatch in DataPlane";
}


void DataPlane::PacketFunctor::operator()(EthernetPacket* const pkt,
                                          const Interface::PtrConst iface) {
  DLOG << "EthernetPacket dispatch in DataPlane";
  DLOG << "  iface: " << iface->name();
  DLOG << "  src: " << pkt->src();
  DLOG << "  dst: " << pkt->dst();
  DLOG << "  type: " << pkt->typeName();

  // Dispatch encapsulated packet.
  Packet::Ptr payload_pkt = pkt->payload();
  (*payload_pkt)(this, iface);
}


void DataPlane::PacketFunctor::operator()(ICMPPacket* const pkt,
                                          const Interface::PtrConst iface) {

}


void DataPlane::PacketFunctor::operator()(IPPacket* const pkt,
                                          const Interface::PtrConst iface) {
  DLOG << "IPPacket dispatch in DataPlane";
}


void DataPlane::PacketFunctor::operator()(UnknownPacket* const pkt,
                                          const Interface::PtrConst iface) {

}
