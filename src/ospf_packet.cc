#include "ospf_packet.h"

#include <netinet/in.h>

#include "fwk/log.h"

#include "interface.h"
#include "packet_buffer.h"

/* Static global log instance */
static Fwk::Log::Ptr log_ = Fwk::Log::LogNew("OSPFPacket");


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
  struct ospf_pkt ospf_pkt;
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

const uint8_t OSPFPacket::kVersion;

const size_t OSPFHelloPacket::kPacketSize = sizeof(struct ospf_hello_pkt);

const uint8_t OSPFLSUPacket::kDefaultTTL;
const size_t OSPFLSUPacket::kHeaderSize = sizeof(struct ospf_lsu_hdr);

const size_t OSPFLSUAdvPacket::kSize = sizeof(struct ospf_lsu_adv);


/* OSPFPacket */

OSPFPacket::Ptr
OSPFPacket::NewDefault(PacketBuffer::Ptr buffer,
                       const RouterID& router_id,
                       const AreaID& area_id,
                       OSPFType packet_type,
                       uint16_t packet_len) {
  return new OSPFPacket(buffer, router_id, area_id, packet_type, packet_len);
}

OSPFPacket::OSPFPacket(PacketBuffer::Ptr buffer, unsigned int buffer_offset)
    : Packet(buffer, buffer_offset),
      ospf_pkt_((struct ospf_pkt*)offsetAddress(0)) {}

OSPFPacket::OSPFPacket(PacketBuffer::Ptr buffer,
                       const RouterID& router_id,
                       const AreaID& area_id,
                       OSPFType packet_type,
                       uint16_t packet_len)
    : Packet(buffer, buffer->size() - packet_len),
      ospf_pkt_((struct ospf_pkt*)offsetAddress(0)) {
  versionIs(OSPFPacket::kVersion);
  typeIs(packet_type);
  packetLengthIs(packet_len);
  routerIDIs(router_id);
  areaIDIs(area_id);
  autypeAndAuthAreZero();
}

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
OSPFPacket::packetLength() const {
  return ntohs(ospf_pkt_->len);
}

void
OSPFPacket::packetLengthIs(uint16_t len) {
  ospf_pkt_->len = htons(len);
}

RouterID
OSPFPacket::routerID() const {
  return ntohl(ospf_pkt_->router_id);
}

void
OSPFPacket::routerIDIs(const RouterID& id) {
  ospf_pkt_->router_id = htonl(id.value());
}

AreaID
OSPFPacket::areaID() const {
  return ntohl(ospf_pkt_->area_id);
}

void
OSPFPacket::areaIDIs(const AreaID& id) {
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

void
OSPFPacket::autypeAndAuthAreZero() {
  ospf_pkt_->autype = 0x0;
  ospf_pkt_->auth = 0x0;
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

  pkt->enclosingPacketIs(this->enclosingPacket());
  return pkt;
}

bool
OSPFPacket::valid() const {
  if (len() < sizeof(struct ospf_pkt)) {
    DLOG << "Packet is smaller than a base OSPF packet.";
    return false;
  }

  if (len() != packetLength()) {
    DLOG << "Packet length is incorrect.";
    DLOG << "  expected: " << packetLength();
    DLOG << "  actual:   " << len();
    return false;
  }

  if (version() != kVersion) {
    DLOG << "Unsupported OSPF version.";
    return false;
  }

  if (ospf_pkt_->autype != 0 || ospf_pkt_->auth != 0) {
    DLOG << "Autype or auth fields are not zero.";
    return false;
  }

  if (ospf_pkt_->type != kHello && ospf_pkt_->type != kLSU) {
    DLOG << "Invalid value in type field.";
    return false;
  }

  if (!checksumValid()) {
    DLOG << "Invalid checksum.";
    return false;
  }

  return true;
}

void
OSPFPacket::operator()(Functor* const f, const Interface::PtrConst iface) {
  (*f)(this, iface);
}


/* OSPFHelloPacket */

const IPv4Addr OSPFHelloPacket::kBroadcastAddr(0xe0000005);

OSPFHelloPacket::Ptr
OSPFHelloPacket::NewDefault(PacketBuffer::Ptr buffer,
                            const RouterID& router_id,
                            const AreaID& area_id,
                            const IPv4Addr& mask,
                            uint16_t helloint) {
  return new OSPFHelloPacket(buffer, router_id, area_id, mask, helloint);
}

OSPFHelloPacket::OSPFHelloPacket(PacketBuffer::Ptr buffer,
                                 unsigned int buffer_offset)
    : OSPFPacket(buffer, buffer_offset),
      ospf_hello_pkt_((struct ospf_hello_pkt*)offsetAddress(0)) {}

OSPFHelloPacket::OSPFHelloPacket(PacketBuffer::Ptr buffer,
                                 const RouterID& router_id,
                                 const AreaID& area_id,
                                 const IPv4Addr& mask,
                                 uint16_t helloint)
    : OSPFPacket(buffer, router_id, area_id, OSPFPacket::kHello, kPacketSize),
      ospf_hello_pkt_((struct ospf_hello_pkt*)offsetAddress(0)) {
  subnetMaskIs(mask);
  hellointIs(helloint);
  paddingIsZero();
  checksumReset();
}

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
OSPFHelloPacket::paddingIsZero() {
  ospf_hello_pkt_->padding = 0x0;
}

bool
OSPFHelloPacket::valid() const {
  if (!OSPFPacket::valid())
    return false;

  if (len() < sizeof(struct ospf_hello_pkt)) {
    DLOG << "Packet is smaller than a standard OSPF hello packet";
    return false;
  }

  if (ospf_hello_pkt_->padding != 0) {
    DLOG << "Padding is not zero.";
    return false;
  }

  IPPacket::Ptr ip_pkt = Ptr::st_cast<IPPacket>(enclosingPacket());
  if (ip_pkt) {
    if (ip_pkt->dst() != kBroadcastAddr) {
      DLOG << "Destination address is not the broadcast address";
      return false;
    }
  } else {
    WLOG << "No validation done on the IP header. Enclosing packet is NULL.";
  }

  return true;
}

void
OSPFHelloPacket::operator()(Functor* const f,
                            const Interface::PtrConst iface) {
  (*f)(this, iface);
}


/* OSPFLSUPacket */

OSPFLSUPacket::Ptr
OSPFLSUPacket::NewDefault(PacketBuffer::Ptr buffer,
                          const RouterID& router_id,
                          const AreaID& area_id,
                          uint32_t adv_count,
                          uint16_t lsu_seqno) {
  return new OSPFLSUPacket(buffer, router_id, area_id, adv_count, lsu_seqno);
}

OSPFLSUPacket::OSPFLSUPacket(PacketBuffer::Ptr buffer,
                             unsigned int buffer_offset)
    : OSPFPacket(buffer, buffer_offset),
      ospf_lsu_hdr_((struct ospf_lsu_hdr*)offsetAddress(0)) {}

OSPFLSUPacket::OSPFLSUPacket(PacketBuffer::Ptr buffer,
                             const RouterID& router_id,
                             const AreaID& area_id,
                             uint32_t adv_count,
                             uint16_t lsu_seqno)
    : OSPFPacket(buffer, router_id, area_id, OSPFPacket::kLSU,
                 OSPFLSUPacket::kHeaderSize +
                 adv_count * OSPFLSUAdvPacket::kSize),
      ospf_lsu_hdr_((struct ospf_lsu_hdr*)offsetAddress(0)) {
  seqnoIs(lsu_seqno);
  ttlIs(OSPFLSUPacket::kDefaultTTL);
  advCountIs(adv_count);
}

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
OSPFLSUPacket::ttlDec(uint16_t delta) {
  ttlIs(ttl() - delta);
}

uint32_t
OSPFLSUPacket::advCount() const {
  return ntohl(ospf_lsu_hdr_->adv_count);
}

void
OSPFLSUPacket::advCountIs(uint32_t count) {
  ospf_lsu_hdr_->adv_count = htonl(count);
}

OSPFLSUAdvPacket::Ptr
OSPFLSUPacket::advertisement(uint32_t index) {
  unsigned int off = bufferOffset() +
                     OSPFLSUPacket::kHeaderSize +
                     index * OSPFLSUAdvPacket::kSize;
  return OSPFLSUAdvPacket::New(buffer(), off);
}

OSPFLSUAdvPacket::PtrConst
OSPFLSUPacket::advertisement(uint32_t index) const {
  OSPFLSUPacket* self = const_cast<OSPFLSUPacket*>(this);
  return self->advertisement(index);
}

bool
OSPFLSUPacket::valid() const {
  if (!OSPFPacket::valid())
    return false;

  size_t required_mem = sizeof(struct ospf_lsu_hdr) +
                        advCount() * sizeof(struct ospf_lsu_adv);
  if (len() < required_mem) {
    DLOG << "Packet buffer too small to accommodate "
         << "stated number of LSU advertisements.";
    return false;
  }

  return true;
}

void
OSPFLSUPacket::operator()(Functor* const f, const Interface::PtrConst iface) {
  (*f)(this, iface);
}


/* OSPFLSUAdvPacket */

OSPFLSUAdvPacket::OSPFLSUAdvPacket(PacketBuffer::Ptr buffer,
                                   unsigned int buffer_offset)
    : Packet(buffer, buffer_offset),
      ospf_lsu_adv_((struct ospf_lsu_adv*)offsetAddress(0)) {}

IPv4Addr
OSPFLSUAdvPacket::subnet() const {
  return ntohl(ospf_lsu_adv_->subnet);
}

void
OSPFLSUAdvPacket::subnetIs(const IPv4Addr& subnet) {
  ospf_lsu_adv_->subnet = subnet.nbo();
}

IPv4Addr
OSPFLSUAdvPacket::subnetMask() const {
  return ntohl(ospf_lsu_adv_->mask);
}

void
OSPFLSUAdvPacket::subnetMaskIs(const IPv4Addr& mask) {
  ospf_lsu_adv_->mask = mask.nbo();
}

RouterID
OSPFLSUAdvPacket::routerID() const {
  return ntohl(ospf_lsu_adv_->router_id);
}

void
OSPFLSUAdvPacket::routerIDIs(const RouterID& id) {
  ospf_lsu_adv_->router_id = htonl(id.value());
}

void
OSPFLSUAdvPacket::operator()(Functor* const f,
                             const Interface::PtrConst iface) {
  (*f)(this, iface);
}
