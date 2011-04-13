#ifndef ICMP_PACKET_H_7H1D49WV
#define ICMP_PACKET_H_7H1D49WV

#include <string>

#include "fwk/buffer.h"

#include "ip_packet.h"
#include "packet.h"

class Interface;


class ICMPPacket : public Packet {
 public:
  typedef Fwk::Ptr<const ICMPPacket> PtrConst;
  typedef Fwk::Ptr<ICMPPacket> Ptr;

  enum Type {
    kEchoReply = 0,
    kDestUnreachable = 3,
    kEchoRequest = 8,
    kTimeExceeded = 11
  };

  static Ptr ICMPPacketNew(Fwk::Buffer::Ptr buffer,
                           unsigned int buffer_offset) {
    return new ICMPPacket(buffer, buffer_offset);
  }

  // Double-dispatch support.
  virtual void operator()(Functor* f, Fwk::Ptr<const Interface> iface);

  // Returns the ICMP Type.
  Type type() const;

  // Sets the ICMP Type.
  void typeIs(Type t);

  // Returns the Type name.
  std::string typeName() const;

  // Returns the ICMP Code (subtype).
  uint8_t code() const;

  // Sets the ICMP Code (subtype).
  void codeIs(uint8_t code);

  // Returns ICMP header+data checksum.
  uint16_t checksum() const;

  // Sets the ICMP header+data checksum.
  void checksumIs(uint16_t ck);

  // Recomputes the checksum of the ICMP header+data.
  void checksumReset();

  // Header length in bytes.
  static const int kHeaderLen = 8;

 protected:
  struct ICMPHeader {
    uint8_t type;
    uint8_t code;
    uint16_t cksum;
    uint32_t rest;
  } __attribute__((packed));

  ICMPPacket(Fwk::Buffer::Ptr buffer, unsigned int buffer_offset);
  uint16_t computeChecksum() const;

  struct ICMPHeader* icmp_hdr_;
};


class ICMPTimeExceededPacket : public ICMPPacket {
 public:
  typedef Fwk::Ptr<const ICMPTimeExceededPacket> PtrConst;
  typedef Fwk::Ptr<ICMPTimeExceededPacket> Ptr;

  static Ptr New(Fwk::Buffer::Ptr buffer,
                 unsigned int buffer_offset) {
    return new ICMPTimeExceededPacket(buffer, buffer_offset);
  }
  static Ptr New(ICMPPacket::Ptr icmp_pkt) {
    return new ICMPTimeExceededPacket(icmp_pkt);
  }

  // Double-dispatch support.
  virtual void operator()(Functor* f, Fwk::Ptr<const Interface> iface);

  // Sets the original packet that generated this message.
  void originalPacketIs(IPPacket::Ptr pkt);

 protected:
  ICMPTimeExceededPacket(Fwk::Buffer::Ptr buffer, unsigned int buffer_offset);
  ICMPTimeExceededPacket(ICMPPacket::Ptr icmp_pkt);
};


#endif
