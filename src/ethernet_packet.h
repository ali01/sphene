#ifndef ETHERNET_PACKET_H_5KIE50BV
#define ETHERNET_PACKET_H_5KIE50BV

#include <inttypes.h>
#include <net/ethernet.h>
#include <string>

#include "fwk/buffer.h"
#include "fwk/ptr.h"

#include "packet.h"


typedef uint16_t EthernetType;

class EthernetAddr {
 public:
  EthernetAddr();
  EthernetAddr(const uint8_t addr[ETH_ALEN]);

  bool operator==(const EthernetAddr& other) const;

  operator std::string() const;

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

  EthernetAddr src() const;
  void srcIs(const EthernetAddr& src);

  EthernetAddr dst() const;
  void dstIs(const EthernetAddr& dst);

  EthernetType type() const;
  void typeIs(const EthernetType eth_type);

  std::string typeName() const;

  Packet::Ptr payload() const;

 protected:
  EthernetPacket(Fwk::Buffer::Ptr buffer, unsigned int buffer_offset);

  struct ether_header* eth_hdr;
};

#endif
