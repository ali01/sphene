#ifndef GRE_PACKET_H_
#define GRE_PACKET_H_

#include <string>

#include "ethernet_packet.h"
#include "packet.h"
#include "packet_buffer.h"

class Interface;


class GREPacket : public Packet {
 public:
  typedef Fwk::Ptr<const GREPacket> PtrConst;
  typedef Fwk::Ptr<GREPacket> Ptr;

  static const size_t kHeaderSize;
  static const size_t kHeaderSizeWithoutChecksum;

  static Ptr GREPacketNew(PacketBuffer::Ptr buffer,
                          unsigned int buffer_offset) {
    return new GREPacket(buffer, buffer_offset);
  }

  virtual void operator()(Functor* f, Fwk::Ptr<const Interface> iface);

  // Packet validation
  virtual bool valid() const;

  // Returns true if the C (checksum) bit) is enabled. Per RFC2784, this bit
  // determines the presence of the Checksum and Reserved1 fields.
  bool checksumPresent() const;

  // Sets the C (checksum) bit. Per RFC2784, this bit determines the presence
  // of the Checksum and Reserved1 fields.
  void checksumPresentIs(bool value);

  // Returns the checksum in the Checksum field or 0 if the checksum is not
  // present.
  uint16_t checksum() const;

  // Sets the checksum value in the packet. Does nothing if the C (checksum)
  // bit is not enabled.
  void checksumIs(uint16_t ck);

  // Returns true if the checksum in the packet is correct, or the checksum
  // field is not present.
  bool checksumValid() const;

  // Recomputes the checksum and updates the packet. Does nothing if the C
  // (checksum) bit is not enabled. Returns the computed checksum or zero if
  // the C bit is not enabled.
  uint16_t checksumReset();

  // Returns the value in the Reserved0 field of the packet.
  uint16_t reserved0() const;

  // Sets the Reserved0 field.
  void reserved0Is(uint16_t value);

  // Returns the version value.
  uint8_t version() const;

  // Sets the version value.
  void versionIs(uint8_t value);

  // Returns the encapsulated protocol type.
  EthernetPacket::EthernetType protocol() const;

  // Sets the encapsulated protocol type.
  void protocolIs(EthernetPacket::EthernetType ptype);

  // Returns the value in the Reserved1 field of the packet, or zero if the
  // checksum bit is not enabled.
  uint16_t reserved1() const;

  // Sets the Reserved1 field. Does nothing if the checksum bit is not enabled.
  void reserved1Is(uint16_t value);

  // Returns the encapsulated packet.
  Packet::Ptr payload();

 protected:
  GREPacket(PacketBuffer::Ptr buffer, unsigned int buffer_offset);

 private:
  struct GREHeader {
    uint16_t c_resv0_ver;  // C bit, Reserved0, Ver
    uint16_t ptype;        // protocol type
    uint16_t cksum;        // checksum of header and payload (optional)
    uint16_t resv1;        // Reserved1 (optional)
  } __attribute__((packed));

  uint16_t computeChecksum() const;

  struct GREHeader* gre_hdr_;
};

#endif
