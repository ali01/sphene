#ifndef ARP_PACKET_H_WDIREBCF
#define ARP_PACKET_H_WDIREBCF

#include "fwk/buffer.h"

#include "ethernet_packet.h"
#include "ip_packet.h"
#include "packet.h"

class Interface;


class ARPPacket : public Packet {
 public:
  typedef Fwk::Ptr<const ARPPacket> PtrConst;
  typedef Fwk::Ptr<ARPPacket> Ptr;

  enum Operation {
    kRequest = 1,
    kReply   = 2
  };

  // Only IPv4-on-Ethernet packets are supported.
  static Ptr ARPPacketNew(Fwk::Buffer::Ptr buffer,
                          unsigned int buffer_offset) {
    return new ARPPacket(buffer, buffer_offset);
  }

  virtual void operator()(Functor* f, Fwk::Ptr<const Interface> iface);

  // Returns the operation of the ARP packet.
  Operation operation() const;

  // Sets the operation of the ARP packet.
  void operationIs(const Operation& op);

  // Returns the sender hardware address.
  EthernetAddr senderHWAddr() const;

  // Sets the sender hardware address.
  void senderHWAddrIs(const EthernetAddr& addr);

  // Returns the target hardware address.
  EthernetAddr targetHWAddr() const;

  // Sets the target hardware address.
  void targetHWAddrIs(const EthernetAddr& addr);

  // Returns the sender protocol address.
  IPv4Addr senderPAddr() const;

  // Sets the sender protocol address.
  void senderPAddrIs(const IPv4Addr& addr);

  // Returns the target protocol address.
  IPv4Addr targetPAddr() const;

  // Sets the sender protocol address.
  void targetPAddrIs(const IPv4Addr& addr);

  // Packet length in bytes.
  static const int kPacketLen = 28;

 protected:
  ARPPacket(Fwk::Buffer::Ptr buffer, unsigned int buffer_offset);

 private:
  struct ARPHeader {
    uint16_t htype;                        // hardware type
    uint16_t ptype;                        // protocol type
    uint8_t  hlen;                         // hardware address length
    uint8_t  plen;                         // protocol address length
    uint16_t oper;                         // ARP operation
    uint8_t  sha[EthernetAddr::kAddrLen];  // sender hardware address
    uint32_t spa;                          // sender protocol address
    uint8_t  tha[EthernetAddr::kAddrLen];  // target hardware address
    uint32_t tpa;                          // target protocol address
  } __attribute__((packed));

  struct ARPHeader* arp_hdr_;
};

#endif
