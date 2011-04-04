#include "data_plane.h"

#include <string>
#include "fwk/named_interface.h"

#include "arp_packet.h"
#include "ethernet_packet.h"
#include "icmp_packet.h"
#include "ip_packet.h"

DataPlane::DataPlane(const std::string& name) : Fwk::NamedInterface(name) { }

void DataPlane::PacketFunctor::operator()(ARPPacket* pkt) { }

void DataPlane::PacketFunctor::operator()(EthernetPacket* pkt) { }

void DataPlane::PacketFunctor::operator()(ICMPPacket* pkt) { }

void DataPlane::PacketFunctor::operator()(IPPacket* pkt) { }
