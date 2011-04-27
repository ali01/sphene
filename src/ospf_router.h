#ifndef OSPF_ROUTER_H_LFORNADU
#define OSPF_ROUTER_H_LFORNADU

#include "fwk/log.h"
#include "fwk/ptr_interface.h"

#include "interface.h"
#include "ospf_packet.h"
#include "ospf_interface_map.h"


/* Forward declarations. */
class Interface;


class OSPFRouter : public Fwk::PtrInterface<OSPFRouter> {
 public:
  typedef Fwk::Ptr<const OSPFRouter> PtrConst;
  typedef Fwk::Ptr<OSPFRouter> Ptr;

  static const uint16_t kDefaultHelloInterval = 10;

  static Ptr New(uint32_t router_id, uint32_t area_id) {
    return new OSPFRouter(router_id, area_id);
  }

  void packetNew(Packet::Ptr pkt, Interface::PtrConst iface);

  uint32_t routerID() const { return router_id_; }
  uint32_t areaID() const { return area_id_; }

  OSPFInterfaceMap::Ptr interfaceMap() { return interfaces_; }
  OSPFInterfaceMap::PtrConst interfaceMap() const { return interfaces_; }

 protected:
  OSPFRouter(uint32_t router_id, uint32_t area_id);

 private:
  class PacketFunctor : public Packet::Functor {
   public:
    PacketFunctor(OSPFRouter* ospf_router);

    void operator()(OSPFPacket*, Fwk::Ptr<const Interface>);
    void operator()(OSPFHelloPacket*, Fwk::Ptr<const Interface>);
    void operator()(OSPFLSUAdvertisement*, Fwk::Ptr<const Interface>);
    void operator()(OSPFLSUPacket*, Fwk::Ptr<const Interface>);

   private:
    OSPFRouter* ospf_router_;
    OSPFInterfaceMap* interfaces_;
    Fwk::Log::Ptr log_;
  };

  /* data members */
  Fwk::Log::Ptr log_;
  PacketFunctor functor_;

  uint32_t router_id_;
  uint32_t area_id_;
  OSPFInterfaceMap::Ptr interfaces_;

  /* operations disallowed */
  OSPFRouter(const OSPFRouter&);
  void operator=(const OSPFRouter&);
};

#endif
