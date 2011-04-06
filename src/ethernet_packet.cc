#include "ethernet_packet.h"

#include <arpa/inet.h>
#include <cstdio>
#include <cstring>
#include <net/ethernet.h>
#include <string>

#include "arp_packet.h"
#include "fwk/buffer.h"
#include "fwk/exception.h"
#include "ip_packet.h"
#include "packet.h"
#include "unknown_packet.h"


EthernetAddr::EthernetAddr() {
  memset(addr_, 0, sizeof(addr_));
}


EthernetAddr::EthernetAddr(const uint8_t addr[ETH_ALEN]) {
  memcpy(addr_, addr, ETH_ALEN);
}


bool EthernetAddr::operator==(const EthernetAddr& other) const {
  return (memcmp(addr_, other.addr_, ETH_ALEN) == 0);
}


EthernetAddr::operator std::string() const {
  char mac[17];
  sprintf(mac, "%02x:%02x:%02x:%02x:%02x:%02x",
          addr_[0], addr_[1], addr_[2], addr_[3], addr_[4], addr_[5]);
  return mac;
}


EthernetPacket::EthernetPacket(Fwk::Buffer::Ptr buffer,
                               unsigned int buffer_offset)
    : Packet(buffer, buffer_offset) {
  unsigned int length = buffer->size() - buffer_offset;
  if (length < ETHER_HDR_LEN)
    throw Fwk::RangeException("EthernetPacket", "packet length too small");
  eth_hdr = (struct ether_header*)((uint8_t*)buffer->data() + buffer_offset);
}


EthernetAddr EthernetPacket::src() const {
  return (EthernetAddr&)eth_hdr->ether_shost;
}


void EthernetPacket::srcIs(const EthernetAddr& src) {
  (EthernetAddr&)eth_hdr->ether_shost = src;
}


EthernetAddr EthernetPacket::dst() const {
  return (EthernetAddr&)eth_hdr->ether_dhost;
}


void EthernetPacket::dstIs(const EthernetAddr& dst) {
  (EthernetAddr&)eth_hdr->ether_dhost = dst;
}


EthernetType EthernetPacket::type() const {
  return ntohs(eth_hdr->ether_type);
}


void EthernetPacket::typeIs(const EthernetType eth_type) {
  eth_hdr->ether_type = htons(eth_type);
}


std::string EthernetPacket::typeName() const {
  switch (type()) {
    case ETHERTYPE_ARP:
      return "ARP";
      break;
    case ETHERTYPE_IP:
      return "IP";
      break;
    default:
      return "unknown";
  }
}


Packet::Ptr EthernetPacket::payload() const {
  uint16_t payload_offset = ETHER_HDR_LEN;

  switch (type()) {
    case ETHERTYPE_ARP:
      return ARPPacket::ARPPacketNew(buffer_, payload_offset);
      break;
    case ETHERTYPE_IP:
      return IPPacket::IPPacketNew(buffer_, payload_offset);
      break;
    default:
      return UnknownPacket::UnknownPacketNew(buffer_, payload_offset);
      break;
  }
}
