#ifndef ETHERNET_PACKET_H_5KIE50BV
#define ETHERNET_PACKET_H_5KIE50BV

#include <inttypes.h>
#include <net/ethernet.h>
#include <string>

#include "fwk/buffer.h"
#include "fwk/ptr.h"

#include "packet.h"


class EthernetAddr {
 public:
  // Construct EthernetAddr of '00:00:00:00:00:00'.
  EthernetAddr();

  // Construct EthernetAddr from 'addr'.
  // '*addr' must be at least 'kAddrLen' bytes.
  EthernetAddr(const uint8_t* addr);

  // Construct EthernetAddr from a string like 'C0:FF:EE:BA:BE:EE'. Invalid
  // strings will set the MAC to all zeroes.
  EthernetAddr(const std::string& str);
  EthernetAddr(const char* str);

  // Copy constructor.
  bool operator==(const EthernetAddr& other) const;

  // Stringify the EthernetAddr into something like 'DE:AD:BE:EF:BA:BE'.
  operator std::string() const;

  // Address length in bytes.
  static const int kAddrLen = 6;

 protected:
  void init(const std::string& str);

  uint8_t addr_[kAddrLen];
};


class EthernetPacket : public Packet {
 public:
  typedef Fwk::Ptr<const EthernetPacket> PtrConst;
  typedef Fwk::Ptr<EthernetPacket> Ptr;

  enum EthernetType {
    kIP      = ETHERTYPE_IP,
    kARP     = ETHERTYPE_ARP,
    kUnknown = 0
  };

  // Construct a new EthernetPacket in 'buffer' starting at 'buffer_offset'
  // within the buffer.
  static Ptr EthernetPacketNew(Fwk::Buffer::Ptr buffer,
                               unsigned int buffer_offset) {
    return new EthernetPacket(buffer, buffer_offset);
  }

  // Functor for double-dispatch.
  virtual void operator()(Functor* f) {
    (*f)(this);
  }

  // Returns the source address.
  EthernetAddr src() const;

  // Sets the source address to 'src'.
  void srcIs(const EthernetAddr& src);

  // Returns the destination address.
  EthernetAddr dst() const;

  // Sets the destination address to 'dst'.
  void dstIs(const EthernetAddr& dst);

  // Returns the EtherType (ARP, IP, ...).
  EthernetType type() const;

  // Sets the EtherType to 'eth_type'.
  void typeIs(const EthernetType eth_type);

  // Returns a string name for the EtherType ("ARP", "IP", ...).
  std::string typeName() const;

  // Returns the encapsulated packet.
  Packet::Ptr payload() const;

 protected:
  // Constructs an EthernetPacket from 'buffer' starting at 'buffer_offset'
  // within the buffer.
  EthernetPacket(Fwk::Buffer::Ptr buffer, unsigned int buffer_offset);

  struct ether_header* eth_hdr;
};

#endif
