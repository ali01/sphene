#ifndef GRE_PACKET_H_
#define GRE_PACKET_H_

#include <string>

#include "fwk/buffer.h"

#include "packet.h"

class Interface;


class GREPacket : public Packet {
 public:
  typedef Fwk::Ptr<const GREPacket> PtrConst;
  typedef Fwk::Ptr<GREPacket> Ptr;

  static const size_t kHeaderSize;

  static Ptr GREPacketNew(Fwk::Buffer::Ptr buffer,
                          unsigned int buffer_offset) {
    return new GREPacket(buffer, buffer_offset);
  }

  virtual void operator()(Functor* f, Fwk::Ptr<const Interface> iface);


 protected:
  GREPacket(Fwk::Buffer::Ptr buffer, unsigned int buffer_offset);

 private:
  struct GREHeader {
    uint8_t crkss_recur;  // C, R, K, S, s bits & recur field
    uint8_t flags_ver;    // flags and version
    uint16_t ptype;       // protocol type
    uint16_t cksum;       // checksum of header and payload
    uint16_t offset;      // offset field
    uint32_t key;         // key
    uint32_t seqno;       // sequence number
    uint32_t routing;     // routing field -- deprecated by RFC 2784
  } __attribute__((packed));

  struct GREHeader* gre_hdr_;
};

#endif
