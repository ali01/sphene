#ifndef ETHERNET_PACKET_H_5KIE50BV
#define ETHERNET_PACKET_H_5KIE50BV

#include <cstring>
#include <net/ethernet.h>
#include <inttypes.h>

#include "fwk/buffer.h"

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
  EthernetPacket(Fwk::Buffer::Ptr buffer, unsigned int buffer_offset)
      : Packet(buffer, buffer_offset) {
    eth_hdr = (struct ether_header*)buffer->data();
  }

  /* Functor for double-dispatch. */
  virtual void operator()(Functor* f) {
    (*f)(this);
  }

  EthernetAddr src() const {
    return (EthernetAddr&)eth_hdr->ether_shost;
  }

  EthernetAddr dst() const {
    return (EthernetAddr&)eth_hdr->ether_dhost;
  }

  EthernetType type() const {
    return eth_hdr->ether_type;
  }

  void typeIs(const EthernetType eth_type) {
    eth_hdr->ether_type = eth_type;
  }

 protected:
  struct ether_header* eth_hdr;
};

#endif
