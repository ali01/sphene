#include "ethernet_packet.h"

#include <arpa/inet.h>
#include <cstdio>
#include <cstring>
#include <net/ethernet.h>
#include <string>

#include "arp_packet.h"
#include "fwk/exception.h"
#include "interface.h"
#include "ip_packet.h"
#include "packet.h"
#include "packet_buffer.h"
#include "unknown_packet.h"

// Minimum size of string buffers for formatting addresses.
static const int kAddrStrLen = 18;

const EthernetAddr EthernetAddr::kBroadcast("FF:FF:FF:FF:FF:FF");
const EthernetAddr EthernetAddr::kZero;
const int EthernetAddr::kAddrLen;

const size_t EthernetPacket::kHeaderSize;
const size_t EthernetPacket::kMTU = 1500;


EthernetAddr::EthernetAddr() {
  memset(addr_, 0, kAddrLen);
}


EthernetAddr::EthernetAddr(const uint8_t* const addr) {
  memcpy(addr_, addr, kAddrLen);
}


EthernetAddr::EthernetAddr(const std::string& str) {
  init(str);
}


EthernetAddr::EthernetAddr(const char* const str) {
  init(str);
}


void EthernetAddr::init(const std::string& str) {
  // TODO(ms): Throw exception here?
  if (str.size() != kAddrStrLen - 1)  // -1 for NULL terminator
    goto error;

  // Try uppercase first.
  if (sscanf(str.c_str(), "%02X:%02X:%02X:%02X:%02X:%02X",
             (unsigned int*)(&addr_[0]),
             (unsigned int*)(&addr_[1]),
             (unsigned int*)(&addr_[2]),
             (unsigned int*)(&addr_[3]),
             (unsigned int*)(&addr_[4]),
             (unsigned int*)(&addr_[5])) != kAddrLen) {
    // Try lowercase.
    if (sscanf(str.c_str(), "%02x:%02x:%02x:%02x:%02x:%02x",
               (unsigned int*)(&addr_[0]),
               (unsigned int*)(&addr_[1]),
               (unsigned int*)(&addr_[2]),
               (unsigned int*)(&addr_[3]),
               (unsigned int*)(&addr_[4]),
               (unsigned int*)(&addr_[5])) != kAddrLen) {
      // Still failed to find a valid MAC.
      goto error;
    }
  }

  return;
error:
  // Something went wrong. Clear address.
  memset(addr_, 0, kAddrLen);
}


bool EthernetAddr::operator==(const EthernetAddr& other) const {
  return (memcmp(addr_, other.addr_, ETH_ALEN) == 0);
}


EthernetAddr::operator std::string() const {
  char mac[18];
  sprintf(mac, "%02X:%02X:%02X:%02X:%02X:%02X",
          addr_[0], addr_[1], addr_[2], addr_[3], addr_[4], addr_[5]);
  return mac;
}


EthernetPacket::EthernetPacket(const PacketBuffer::Ptr buffer,
                               const unsigned int buffer_offset)
    : Packet(buffer, buffer_offset),
      eth_hdr((struct ether_header*)offsetAddress(0)) { }


void EthernetPacket::operator()(Functor* const f,
                                const Interface::PtrConst iface) {
  (*f)(this, iface);
}


bool EthernetPacket::valid() const {
  // Verify length.
  if (len() < kHeaderSize)
    return false;

  return true;
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


EthernetPacket::EthernetType EthernetPacket::type() const {
  switch (ntohs(eth_hdr->ether_type)) {
    case kARP:
      return kARP;
      break;
    case kIP:
      return kIP;
      break;
    default:
      return kUnknown;
      break;
  }
}


void EthernetPacket::typeIs(const EthernetPacket::EthernetType eth_type) {
  eth_hdr->ether_type = htons(eth_type);
}


std::string EthernetPacket::typeName() const {
  switch (type()) {
    case kARP:
      return "ARP";
      break;
    case kIP:
      return "IP";
      break;
    default:
      return "unknown";
  }
}


Packet::Ptr EthernetPacket::payload() {
  uint16_t payload_offset = bufferOffset() + headerLen();
  Packet::Ptr pkt;

  switch (type()) {
    case kARP:
      pkt = ARPPacket::ARPPacketNew(buffer(), payload_offset);
      break;
    case kIP:
      pkt = IPPacket::IPPacketNew(buffer(), payload_offset);
      break;
    default:
      pkt = UnknownPacket::UnknownPacketNew(buffer(), payload_offset);
      break;
  }

  // Chain the packets together.
  pkt->enclosingPacketIs(this);
  return pkt;
}
