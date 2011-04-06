#ifndef IP_PACKET_H_8CU5IVY3
#define IP_PACKET_H_8CU5IVY3

#include "fwk/buffer.h"

#include "packet.h"

#if __BYTE_ORDER != __LITTLE_ENDIAN && __BYTE_ORDER != __BIG_ENDIAN
#error "Byte ordering not specified."
#endif

/* Typedefs. */
typedef uint8_t IPVersion;
typedef uint8_t IPType;
typedef uint32_t IPv4Addr;
typedef uint8_t IPHeaderLength;
typedef uint8_t IPDiffServices;
typedef uint16_t IPFragmentOffset;
typedef uint8_t IPFlags;

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
  void fragmentOffset(const IPFragmentOffset off);

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
#endif
    uint8_t ip_tos;           /* type of service */
    uint16_t ip_len;          /* total length */
    uint16_t ip_id;           /* identification */
#define IP_RF 0x8000          /* reserved fragment flag */
#define IP_DF 0x4000          /* dont fragment flag */
#define IP_MF 0x2000          /* more fragments flag */
#define IP_OFFMASK 0x1fff     /* mask for fragmenting bits */
#if __BYTE_ORDER == __LITTLE_ENDIAN
    unsigned int ip_off:13;   /* fragment offset field */
    unsigned int ip_fl:3;     /* flags */
#elif __BYTE_ORDER == __BIG_ENDIAN
    unsigned int ip_fl:3;     /* flags */
    unsigned int ip_off:13;   /* fragment offset field */
#endif
    uint8_t ip_ttl;           /* time to live */
    uint8_t ip_p;             /* protocol */
    uint16_t ip_sum;          /* checksum */
    uint32_t ip_src;          /* source address */
    uint32_t ip_dst;          /* destination address */
  } __attribute__ ((packed));
};

#endif
