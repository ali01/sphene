#ifndef ARP_PACKET_H_WDIREBCF
#define ARP_PACKET_H_WDIREBCF

#include <string>

#include "ethernet_packet.h"
#include "ip_packet.h"
#include "packet.h"
#include "packet_buffer.h"

class Interface;


class ARPPacket : public Packet {
 public:
  typedef Fwk::Ptr<const ARPPacket> PtrConst;
  typedef Fwk::Ptr<ARPPacket> Ptr;

  enum HWType {
    kEthernet = 1
  };

  enum PType {
    kIP = 0x0800
  };

  enum Operation {
    kRequest = 1,
    kReply   = 2
  };

  static const size_t kHeaderSize;

  // Only IPv4-on-Ethernet packets are supported.
  static Ptr ARPPacketNew(PacketBuffer::Ptr buffer,
                          unsigned int buffer_offset) {
    return new ARPPacket(buffer, buffer_offset);
  }

  // Packet validation.
  // TODO(ali): implement. Currently throws NotImplementedException.
  virtual bool valid() const;

  virtual void operator()(Functor* f, Fwk::Ptr<const Interface> iface);

  // Returns the hardware type.
  HWType hwType() const;

  // Sets the link layer type.
  void hwTypeIs(HWType hwtype);

  // Returns the protocol type.
  PType pType() const;

  // Sets the protocol type.
  void pTypeIs(PType ptype);

  // Returns the hardware address length.
  uint8_t hwAddrLen() const;

  // Returns the protocol address length.
  uint8_t pAddrLen() const;

  // Returns the operation of the ARP packet.
  Operation operation() const;

  // Sets the operation of the ARP packet.
  void operationIs(const Operation& op);

  // Returns the string name for the operation ('request', 'reply', 'unknown').
  std::string operationName() const;

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
  ARPPacket(PacketBuffer::Ptr buffer, unsigned int buffer_offset);

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
