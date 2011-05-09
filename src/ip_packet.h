#ifndef IP_PACKET_H_8CU5IVY3
#define IP_PACKET_H_8CU5IVY3

#include <string>

#include "fwk/ordinal.h"

#include "ipv4_addr.h"
#include "packet.h"
#include "packet_buffer.h"

/* Typedefs. */
typedef uint8_t IPVersion;
typedef uint8_t IPHeaderLength;
typedef uint8_t IPDiffServices;
typedef uint16_t IPFragmentOffset;
typedef uint8_t IPFlags;

/* Forward declarations. */
class Interface;
struct ip_hdr;


class IPPacket : public Packet {

 public:
  typedef Fwk::Ptr<const IPPacket> PtrConst;
  typedef Fwk::Ptr<IPPacket> Ptr;

  /* IP Header Flags (ip_fl) */
  enum Flags {
    kIP_RF = 0x4, /* reserved fragment flag */
    kIP_DF = 0x2, /* dont fragment flag */
    kIP_MF = 0x1  /* more fragments flag */
  };

  enum IPType {
    kICMP    = 0x01,
    kTCP     = 0x06,
    kUDP     = 0x11,
    kGRE     = 0x2F,
    kOSPF    = 0x59
  };

  static const uint8_t kDefaultTTL = 64;
  static const size_t kHeaderSize;
  static const IPVersion kVersion = 4;

  static Ptr NewDefault(PacketBuffer::Ptr buffer,
                        uint16_t packet_len,
                        IPType protocol,
                        const IPv4Addr& src,
                        const IPv4Addr& dst);

  static Ptr New(PacketBuffer::Ptr buffer, unsigned int buffer_offset) {
    return new IPPacket(buffer, buffer_offset);
  }

  /* Double-dispatch support. */
  virtual void operator()(Functor* f, Fwk::Ptr<const Interface> iface);

  /* Packet validation. */
  virtual bool valid() const;

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
  uint16_t checksumReset();
  bool checksumValid() const;

  // Returns the encapsulated packet.
  Packet::Ptr payload();

  // Returns the IPv4 header length without options.
  uint8_t headerLen() const { return 20; }

  /* accepts any pointer type as PacketBuffer */
  template <typename PacketBuffer>
  static uint16_t compute_cksum(const PacketBuffer* buffer, unsigned int len);

 protected:
  IPPacket(PacketBuffer::Ptr buffer, unsigned int buffer_offset);

 private:
  struct ip_hdr* ip_hdr_;
};

template <typename PacketBuffer>
inline uint16_t
IPPacket::compute_cksum(const PacketBuffer* pkt, unsigned int len) {
  uint32_t sum;
  const uint8_t* buffer = (const uint8_t*)pkt;

  for (sum = 0; len >= 2; buffer += 2, len -= 2)
    sum += buffer[0] << 8 | buffer[1];

  if (len > 0)
    sum += buffer[0] << 8;

  while (sum > 0xFFFF)
    sum = (sum >> 16) + (sum & 0xFFFF);

  sum = ~sum;

  if (sum == 0)
    sum = 0xFFFF;

  return sum;
}

#endif
