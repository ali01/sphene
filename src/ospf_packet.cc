#include "ospf_packet.h"

#include <netinet/in.h>

#include "interface.h"

struct ospf_pkt {
  uint8_t version;          /* protocol version */
  uint8_t type;             /* 1 for hello, 4 for LSU */
  uint16_t len;             /* packet length */
  uint32_t router_id;       /* router ID of the packet's source */
  uint32_t area_id;         /* OSPF area that this packet belongs to */
  uint16_t cksum;           /* IP checksum of the packet excluding auth */
  uint16_t autype;          /* set to zero in PWOSPF */
  uint64_t auth;            /* set to zero in PWOSPF */
} __attribute__((packed));

struct ospf_hello_pkt {
  struct ospf_pkt opsf_pkt;
  uint32_t mask;            /* network mask associated with this interface */
  uint16_t helloint;        /* number of seconds between hello packets */
  uint16_t padding;         /* zero */
} __attribute__((packed));

struct ospf_lsu_hdr {
  struct ospf_pkt ospf_pkt;
  uint16_t seqno;           /* unique seqno associated with this LSU packet */
  uint16_t ttl;             /* hop limited value */
  uint32_t adv_count;       /* number of LSU advertisements that follow */

  /* variable number of link state advertisements */

} __attribute((packed));

/* link state advertisement */
struct ospf_lsu_adv {
  uint32_t subnet;          /* subnet number of advertised route */
  uint32_t mask;            /* subnet mask of advertised route */
  uint32_t router_id;       /* ID of neighboring router on advertised link */
} __attribute((packed));

/* OSPFPacket */

OSPFPacket::OSPFPacket(Fwk::Buffer::Ptr buffer, unsigned int buffer_offset)
    : Packet(buffer, buffer_offset),
      ospf_pkt_((struct ospf_pkt*)offsetAddress(0)) {}

uint8_t
OSPFPacket::version() const {
  return ospf_pkt_->version;
}

void
OSPFPacket::versionIs(uint8_t version) {
  ospf_pkt_->version = version;
}

OSPFPacket::OSPFType
OSPFPacket::type() const {
  return (OSPFType)ospf_pkt_->type;
}

void
OSPFPacket::typeIs(OSPFType type) {
  ospf_pkt_->type = type;
}

uint16_t
OSPFPacket::len() const {
  return ntohs(ospf_pkt_->len);
}

void
OSPFPacket::lenIs(uint16_t len) {
  ospf_pkt_->len = htons(len);
}

uint32_t
OSPFPacket::routerID() const {
  return ntohl(ospf_pkt_->router_id);
}

void
OSPFPacket::routerIDIs(uint32_t id) {
  ospf_pkt_->router_id = htonl(id);
}

uint32_t
OSPFPacket::areaID() const {
  return ntohl(ospf_pkt_->area_id);
}

void
OSPFPacket::areaIDIs(uint32_t id) {
  ospf_pkt_->area_id = htonl(id);
}

uint16_t
OSPFPacket::checksum() const {
  return ntohs(ospf_pkt_->cksum);
}

uint16_t
OSPFPacket::checksumReset() {
  checksumIs(0);
  checksumIs(IPPacket::compute_cksum(ospf_pkt_, len()));
  return checksum();
}

void
OSPFPacket::checksumIs(uint16_t ck) {
  ospf_pkt_->cksum = htons(ck);
}

bool
OSPFPacket::checksumValid() const {
  OSPFPacket *self = const_cast<OSPFPacket*>(this);
  uint16_t pkt_cksum = checksum();
  uint16_t actual_cksum = self->checksumReset();
  self->checksumIs(pkt_cksum);
  return pkt_cksum == actual_cksum;
}

OSPFPacket::Ptr
OSPFPacket::derivedInstance() {
  OSPFPacket::Ptr pkt;

  switch (type()) {
    case kHello:
      pkt = OSPFHelloPacket::New(buffer(), bufferOffset());
      break;
    case kLSU:
      pkt = OSPFLSUPacket::New(buffer(), bufferOffset());
      break;
    default:
      pkt = NULL;
  }

  return pkt;
}

void
OSPFPacket::operator()(Functor* const f, const Interface::PtrConst iface) {
  (*f)(this, iface);
}


/* OSPFHelloPacket */

OSPFHelloPacket::OSPFHelloPacket(Fwk::Buffer::Ptr buffer,
                                 unsigned int buffer_offset)
    : OSPFPacket(buffer, buffer_offset),
      ospf_hello_pkt_((struct ospf_hello_pkt*)offsetAddress(0)) {}

IPv4Addr
OSPFHelloPacket::subnetMask() const {
  return ntohl(ospf_hello_pkt_->mask);
}

void
OSPFHelloPacket::subnetMaskIs(const IPv4Addr& addr) {
  ospf_hello_pkt_->mask = addr.nbo();
}

uint16_t
OSPFHelloPacket::helloint() const {
  return ntohs(ospf_hello_pkt_->helloint);
}

void
OSPFHelloPacket::hellointIs(uint16_t helloint) {
  ospf_hello_pkt_->helloint = htons(helloint);
}

void
OSPFHelloPacket::operator()(Functor* const f,
                            const Interface::PtrConst iface) {
  (*f)(this, iface);
}


/* OSPFLSUPacket */

OSPFLSUPacket::OSPFLSUPacket(Fwk::Buffer::Ptr buffer,
                             unsigned int buffer_offset)
    : OSPFPacket(buffer, buffer_offset),
      ospf_lsu_hdr_((struct ospf_lsu_hdr*)offsetAddress(0)) {}

uint16_t
OSPFLSUPacket::seqno() const {
  return ntohs(ospf_lsu_hdr_->seqno);
}

void
OSPFLSUPacket::seqnoIs(uint16_t seqno) {
  ospf_lsu_hdr_->seqno = htons(seqno);
}

uint16_t
OSPFLSUPacket::ttl() const {
  return ntohs(ospf_lsu_hdr_->ttl);
}

void
OSPFLSUPacket::ttlIs(uint16_t ttl) {
  ospf_lsu_hdr_->ttl = htons(ttl);
}

void
OSPFLSUPacket::operator()(Functor* const f, const Interface::PtrConst iface) {
  (*f)(this, iface);
}