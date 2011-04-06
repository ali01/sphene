#ifndef ETHERNET_PACKET_H_5KIE50BV
#define ETHERNET_PACKET_H_5KIE50BV

#include <arpa/inet.h>
#include <cstdio>
#include <cstring>
#include <inttypes.h>
#include <net/ethernet.h>
#include <string>

#include "fwk/buffer.h"
#include "fwk/exception.h"
#include "fwk/ptr.h"

#include "arp_packet.h"
#include "ip_packet.h"
#include "packet.h"
#include "unknown_packet.h"


typedef uint16_t EthernetType;

class EthernetAddr {
 public:
  EthernetAddr() {
    memset(addr_, 0, sizeof(addr_));
  }

  EthernetAddr(const uint8_t addr[ETH_ALEN]) {
    memcpy(addr_, addr, ETH_ALEN);
  }

  bool operator==(const EthernetAddr& other) const {
    return (memcmp(addr_, other.addr_, ETH_ALEN) == 0);
  }

  operator std::string() const {
    char mac[17];
    sprintf(mac, "%02x:%02x:%02x:%02x:%02x:%02x",
            addr_[0], addr_[1], addr_[2], addr_[3], addr_[4], addr_[5]);
    return mac;
  }

 protected:
  uint8_t addr_[ETH_ALEN];
};


class EthernetPacket : public Packet {
 public:
  typedef Fwk::Ptr<const EthernetPacket> PtrConst;
  typedef Fwk::Ptr<EthernetPacket> Ptr;

  static Ptr EthernetPacketNew(Fwk::Buffer::Ptr buffer,
                               unsigned int buffer_offset) {
    return new EthernetPacket(buffer, buffer_offset);
  }

  /* Functor for double-dispatch. */
  virtual void operator()(Functor* f) {
    (*f)(this);
  }

  EthernetAddr src() const {
    return (EthernetAddr&)eth_hdr->ether_shost;
  }

  void srcIs(const EthernetAddr& src) {
    (EthernetAddr&)eth_hdr->ether_shost = src;
  }

  EthernetAddr dst() const {
    return (EthernetAddr&)eth_hdr->ether_dhost;
  }

  void dstIs(const EthernetAddr& dst) {
    (EthernetAddr&)eth_hdr->ether_dhost = dst;
  }

  EthernetType type() const {
    return ntohs(eth_hdr->ether_type);
  }

  void typeIs(const EthernetType eth_type) {
    eth_hdr->ether_type = htons(eth_type);
  }

  std::string typeName() const {
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

  Packet::Ptr payload() const {
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

 protected:
  EthernetPacket(Fwk::Buffer::Ptr buffer, unsigned int buffer_offset)
      : Packet(buffer, buffer_offset) {
    unsigned int length = buffer->size() - buffer_offset;
    if (length < ETHER_HDR_LEN)
      throw Fwk::RangeException("EthernetPacket", "packet length too small");
    eth_hdr = (struct ether_header*)((uint8_t*)buffer->data() + buffer_offset);
  }

  struct ether_header* eth_hdr;
};

#endif
