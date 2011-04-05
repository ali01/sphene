#ifndef IP_PACKET_H_8CU5IVY3
#define IP_PACKET_H_8CU5IVY3

#include "fwk/buffer.h"

#include "packet.h"

/* Typedefs. */
typedef uint8_t IPVersion;
typedef uint8_t Protocol;
typedef uint32_t IPv4Addr;

class IPPacket : public Packet {

  /* Forward declarations. */
  struct ip_hdr;

 public:
  typedef Fwk::Ptr<const IPPacket> PtrConst;
  typedef Fwk::Ptr<IPPacket> Ptr;

  static Ptr IPPacketNew(Fwk::Buffer::Ptr buffer, unsigned int buffer_offset) {
    return new IPPacket(buffer, buffer_offset);
  }

  IPVersion version() const;
  void versionIs(const IPVersion& version);

  Protocol protocol() const;
  void protocolIs(const Protocol& protocol);

  IPv4Addr src() const;
  void srcIs(const IPv4Addr& src);

  IPv4Addr dst() const;
  void dstIs(const IPv4Addr& dst);

  uint16_t checksum() const;
  void checksumIs(uint16_t ck);
  void checksumReset();

 protected:
  IPPacket(Fwk::Buffer::Ptr buffer, unsigned int buffer_offset);

 private:
  struct ip_hdr* ip_hdr_;

  uint16_t compute_cksum() const;

  /* IP Header packet struct. */
  struct ip_hdr {
#if __BYTE_ORDER == __LITTLE_ENDIAN
    unsigned int ip_hl:4;     /* header length */
    unsigned int ip_v:4;      /* version */
#elif __BYTE_ORDER == __BIG_ENDIAN
    unsigned int ip_v:4;      /* version */
    unsigned int ip_hl:4;     /* header length */
#else
#error "Byte ordering not specified."
#endif
    uint8_t ip_tos;           /* type of service */
    uint16_t ip_len;          /* total length */
    uint16_t ip_id;           /* identification */
    uint16_t ip_off;          /* fragment offset field */
#define IP_RF 0x8000          /* reserved fragment flag */
#define IP_DF 0x4000          /* dont fragment flag */
#define IP_MF 0x2000          /* more fragments flag */
#define IP_OFFMASK 0x1fff     /* mask for fragmenting bits */
    uint8_t ip_ttl;           /* time to live */
    uint8_t ip_p;             /* protocol */
    uint16_t ip_sum;          /* checksum */
    uint32_t ip_src;          /* source address */
    uint32_t ip_dst;          /* destination address */
  } __attribute__ ((packed));
};

#endif
