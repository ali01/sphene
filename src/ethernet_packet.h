#ifndef ETHERNET_PACKET_H_5KIE50BV
#define ETHERNET_PACKET_H_5KIE50BV

#include <cstring>
#include <inttypes.h>
#include <net/ethernet.h>

#include "fwk/buffer.h"
#include "fwk/ptr.h"

#include "arp_packet.h"
#include "packet.h"


typedef uint16_t EthernetType;

class EthernetAddr {
 public:
  EthernetAddr() {
    memset(addr_, 0, sizeof(addr_));
  }

  EthernetAddr(uint8_t addr[ETH_ALEN]) {
    memcpy(addr_, addr, ETH_ALEN);
  }

  bool operator==(const EthernetAddr& other) const {
    return (memcmp(addr_, other.addr_, ETH_ALEN) == 0);
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
    return eth_hdr->ether_type;
  }

  void typeIs(const EthernetType eth_type) {
    eth_hdr->ether_type = eth_type;
  }

  Packet::Ptr payload() const {
    uint16_t payload_offset = ETHER_HDR_LEN;

    switch (type()) {
      case ETHERTYPE_ARP:
        return ARPPacket::ARPPacketNew(buffer_, payload_offset);
        break;
      default:
        return NULL;
        break;
    }
  }

 protected:
  EthernetPacket(Fwk::Buffer::Ptr buffer, unsigned int buffer_offset)
      : Packet(buffer, buffer_offset) {
    eth_hdr = (struct ether_header*)((uint8_t*)buffer->data() + buffer_offset);
  }

  struct ether_header* eth_hdr;
};

#endif
