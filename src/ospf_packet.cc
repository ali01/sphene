#include "ospf_packet.h"

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


void
OSPFPacket::operator()(Functor* const f, const Interface::PtrConst iface) {
  (*f)(this, iface);
}
