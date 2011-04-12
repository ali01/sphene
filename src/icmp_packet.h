#ifndef ICMP_PACKET_H_7H1D49WV
#define ICMP_PACKET_H_7H1D49WV

#include <string>

#include "fwk/buffer.h"

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

 protected:
  ICMPPacket(Fwk::Buffer::Ptr buffer, unsigned int buffer_offset);

  uint16_t computeChecksum() const;

 private:
  struct ICMPHeader {
    uint8_t type;
    uint8_t code;
    uint16_t cksum;
  } __attribute__((packed));

  struct ICMPHeader* icmp_hdr_;
};

#endif
