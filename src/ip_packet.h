#ifndef IP_PACKET_H_8CU5IVY3
#define IP_PACKET_H_8CU5IVY3

#include <string>

#include "fwk/buffer.h"

#include "packet.h"

/* Typedefs. */
typedef uint8_t IPVersion;
typedef uint8_t IPHeaderLength;
typedef uint8_t IPDiffServices;
typedef uint16_t IPFragmentOffset;
typedef uint8_t IPFlags;

/* Forward declarations. */
class Interface;
struct ip_hdr;


class IPv4Addr {
 public:
  IPv4Addr() : addr_(0) {}

  /* addr is expected in network byte order. */
  IPv4Addr(uint32_t addr) : addr_(addr) {}

  /* Construct from the dotted string representation. */
  IPv4Addr(const std::string& addr);
  IPv4Addr(const char* addr);

  bool operator==(const IPv4Addr& other) const {
    return addr_ == other.addr_;
  }

  bool operator==(uint32_t other) const;

  /* Returns IP address in host byte order. */
  operator uint32_t() const;

  /* Returns IP address in network byte order */
  uint32_t nbo() const {
    return addr_;
  }

  /* Returns string representation of IP address. */
  operator std::string() const;

  /* Address length in bytes. */
  static const int kAddrLen = 4;

 protected:
  uint32_t addr_;
};

class IPPacket : public Packet {

 public:
  typedef Fwk::Ptr<const IPPacket> PtrConst;
  typedef Fwk::Ptr<IPPacket> Ptr;

  /* IP Header Flags (ip_fl) */
  enum Flags {
    IP_RF = 0x4, /* reserved fragment flag */
    IP_DF = 0x2, /* dont fragment flag */
    IP_MF = 0x1  /* more fragments flag */
  };

  enum IPType {
    kICMP    = 0x01,
    kTCP     = 0x06,
    kUDP     = 0x11,
    kOSPF    = 0x59
  };

  static Ptr IPPacketNew(Fwk::Buffer::Ptr buffer, unsigned int buffer_offset) {
    return new IPPacket(buffer, buffer_offset);
  }

  /* Double-dispatch support. */
  virtual void operator()(Functor* f, Fwk::Ptr<const Interface> iface);

  IPVersion version() const;
  void versionIs(const IPVersion& version);

  IPHeaderLength headerLength() const;
  void headerLengthIs(const IPHeaderLength& len);

  uint16_t packetLength() const;
  void packetLengthIs(uint16_t len);

  IPDiffServices diffServices() const;
  void diffServicesAre(const IPDiffServices& srv);

  IPType protocol() const;
  void protocolIs(const IPType& protocol);

  uint16_t identification() const;
  void identificationIs(uint16_t id);

  IPFlags flags() const;
  void flagsAre(const IPFlags& flags);

  IPFragmentOffset fragmentOffset() const;
  void fragmentOffsetIs(const IPFragmentOffset& off);

  IPv4Addr src() const;
  void srcIs(const IPv4Addr& src);

  IPv4Addr dst() const;
  void dstIs(const IPv4Addr& dst);

  uint8_t ttl() const;
  void ttlIs(uint8_t ttl);
  void ttlDec(uint8_t dec_amount);

  uint16_t checksum() const;
  void checksumIs(uint16_t ck);
  void checksumReset();

  // Returns the encapsulated packet.
  Packet::Ptr payload() const;

  const uint8_t* buffer() const { return (uint8_t *)ip_hdr_; }

  // Returns the IPv4 header length without options.
  uint8_t headerLen() const { return 20; }

 protected:
  IPPacket(Fwk::Buffer::Ptr buffer, unsigned int buffer_offset);

 private:
  struct ip_hdr* ip_hdr_;

  uint16_t compute_cksum() const;
};

#endif
