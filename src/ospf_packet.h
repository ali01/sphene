#ifndef OSPF_PACKET_H_7R33RO3C
#define OSPF_PACKET_H_7R33RO3C

#include "packet.h"
#include "packet_buffer.h"
#include "ip_packet.h"
#include "ospf_types.h"

// TODO(ali): need tests

/* Forward declarations. */
struct ospf_pkt;
struct ospf_hello_pkt;
struct ospf_lsu_hdr;
struct ospf_lsu_adv;

class OSPFPacket : public Packet {
 public:
  typedef Fwk::Ptr<const OSPFPacket> PtrConst;
  typedef Fwk::Ptr<OSPFPacket> Ptr;

  static const uint8_t kVersion = 2;

  enum OSPFType {
    kHello = 0x1,
    kLSU   = 0x4
  };

  static Ptr New(PacketBuffer::Ptr buffer, unsigned int buffer_offset) {
    return new OSPFPacket(buffer, buffer_offset);
  }

  uint8_t version() const;
  void versionIs(uint8_t version);

  OSPFType type() const;
  void typeIs(OSPFType type);

  uint16_t len() const;
  void lenIs(uint16_t len);

  /* Router ID of the sender of this OSPF Packet. */
  RouterID routerID() const;
  void routerIDIs(const RouterID& id);

  AreaID areaID() const;
  void areaIDIs(const AreaID& id);

  uint16_t checksum() const;
  uint16_t checksumReset();
  void checksumIs(uint16_t ck);
  bool checksumValid() const;

  /* zero autype and auth fields */
  void autypeAndAuthAreZero();

  /* returns an instance of either OSPFHelloPacket or OSPFLSUPacket depending
   * on the value of the packet's type field. */
  virtual OSPFPacket::Ptr derivedInstance();

  /* Packet validation. */
  virtual bool valid() const;

  /* Double-dispatch support. */
  virtual void operator()(Functor* f, Fwk::Ptr<const Interface> iface);

 protected:
  OSPFPacket(PacketBuffer::Ptr buffer, unsigned int buffer_offset);

 private:
  /* Data members. */
  struct ospf_pkt* ospf_pkt_;

  /* Operations disallowed. */
  OSPFPacket(const OSPFPacket&);
  void operator=(const OSPFPacket&);
};


class OSPFHelloPacket : public OSPFPacket {
 public:
  typedef Fwk::Ptr<const OSPFHelloPacket> PtrConst;
  typedef Fwk::Ptr<OSPFHelloPacket> Ptr;

  static const IPv4Addr kBroadcastAddr;
  static const size_t kPacketSize;

  static Ptr New(PacketBuffer::Ptr buffer, unsigned int buffer_offset) {
    return new OSPFHelloPacket(buffer, buffer_offset);
  }

  /* Subnet mask of the interface through which this packet was sent. */
  IPv4Addr subnetMask() const;
  void subnetMaskIs(const IPv4Addr& addr);

  uint16_t helloint() const;
  void hellointIs(uint16_t helloint);

  void paddingIsZero();

  /* Override. */
  virtual OSPFPacket::Ptr derivedInstance() { return this; }

  /* Packet validation. */
  virtual bool valid() const;

  /* Double-dispatch support. */
  virtual void operator()(Functor* f, Fwk::Ptr<const Interface> iface);

 private:
  OSPFHelloPacket(PacketBuffer::Ptr buffer, unsigned int buffer_offset);

  /* Data members. */
  struct ospf_hello_pkt* ospf_hello_pkt_;

  /* Operations disallowed */
  OSPFHelloPacket(const OSPFHelloPacket&);
  void operator=(const OSPFHelloPacket&);
};


class OSPFLSUPacket : public OSPFPacket {
 public:
  typedef Fwk::Ptr<const OSPFLSUPacket> PtrConst;
  typedef Fwk::Ptr<OSPFLSUPacket> Ptr;

  static const size_t kHeaderSize;

  /* OSPFLSUPacket factory constructor. */
  static Ptr New(PacketBuffer::Ptr buffer, unsigned int buffer_offset) {
    return new OSPFLSUPacket(buffer, buffer_offset);
  }

  uint16_t seqno() const;
  void seqnoIs(uint16_t seqno);

  uint16_t ttl() const;
  void ttlIs(uint16_t ttl);
  void ttlDec(uint16_t delta);

  uint32_t advCount() const;
  void advCountIs(uint32_t count);

  Fwk::Ptr<OSPFLSUAdvertisement> advertisement(uint32_t index);
  Fwk::Ptr<const OSPFLSUAdvertisement> advertisement(uint32_t index) const;

  /* Override. */
  virtual OSPFPacket::Ptr derivedInstance() { return this; }

  /* Packet valiation */
  virtual bool valid() const;

  /* Double-dispatch support. */
  virtual void operator()(Functor* f, Fwk::Ptr<const Interface> iface);

 private:
  OSPFLSUPacket(PacketBuffer::Ptr buffer, unsigned int buffer_offset);

  /* Data members. */
  struct ospf_lsu_hdr* ospf_lsu_hdr_;

  /* Operations disallowed */
  OSPFLSUPacket(const OSPFLSUPacket&);
  void operator=(const OSPFLSUPacket&);
};


class OSPFLSUAdvertisement : public Packet {
 public:
  typedef Fwk::Ptr<const OSPFLSUAdvertisement> PtrConst;
  typedef Fwk::Ptr<OSPFLSUAdvertisement> Ptr;

  static const size_t kSize;

  static Ptr New(PacketBuffer::Ptr buffer, unsigned int buffer_offset) {
    return new OSPFLSUAdvertisement(buffer, buffer_offset);
  }

  IPv4Addr subnet() const;
  void subnetIs(const IPv4Addr& subnet);

  IPv4Addr subnetMask() const;
  void subnetMaskIs(const IPv4Addr& mask);

  RouterID routerID() const;
  void routerIDIs(const RouterID& id);

  /* Packet validation. */
  virtual bool valid() const { return true; }

  /* Double-dispatch support. */
  virtual void operator()(Functor* f, Fwk::Ptr<const Interface> iface);

 private:
  OSPFLSUAdvertisement(PacketBuffer::Ptr buffer, unsigned int buffer_offset);

  struct ospf_lsu_adv* ospf_lsu_adv_;

  /* Operations disallowed. */
  OSPFLSUAdvertisement(const OSPFLSUAdvertisement&);
  void operator=(const OSPFLSUAdvertisement&);
};

#endif
