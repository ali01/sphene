#ifndef OSPF_ROUTER_H_LFORNADU
#define OSPF_ROUTER_H_LFORNADU

#include "fwk/log.h"
#include "fwk/ptr_interface.h"

#include "ospf_interface_map.h"
#include "ospf_topology.h"
#include "packet.h"

/* Forward declarations. */
class Interface;
class OSPFLSUPacket;
class OSPFNode;


class OSPFRouter : public Fwk::PtrInterface<OSPFRouter> {
 public:
  typedef Fwk::Ptr<const OSPFRouter> PtrConst;
  typedef Fwk::Ptr<OSPFRouter> Ptr;

  static const uint16_t kDefaultHelloInterval = 10;

  static Ptr New(uint32_t router_id, uint32_t area_id) {
    return new OSPFRouter(router_id, area_id);
  }

  // TODO(ali): perhaps should take OSPFPacket instead of Packet.
  void packetNew(Packet::Ptr pkt, Fwk::Ptr<const Interface> iface);

  uint32_t routerID() const { return router_id_; }
  uint32_t areaID() const { return area_id_; }

  OSPFInterfaceMap::Ptr interfaceMap();
  OSPFInterfaceMap::PtrConst interfaceMap() const;

  OSPFTopology::Ptr topology();
  OSPFTopology::PtrConst topology() const;

 protected:
  OSPFRouter(uint32_t router_id, uint32_t area_id);

 private:
  class PacketFunctor : public Packet::Functor {
   public:
    PacketFunctor(OSPFRouter* ospf_router);

    void operator()(OSPFPacket*, Fwk::Ptr<const Interface>);
    void operator()(OSPFHelloPacket*, Fwk::Ptr<const Interface>);
    void operator()(OSPFLSUPacket*, Fwk::Ptr<const Interface>);

   private:
    OSPFRouter* ospf_router_;
    OSPFInterfaceMap* interfaces_;
    OSPFTopology* topology_;
    Fwk::Log::Ptr log_;
  };

  void process_lsu_advertisements(Fwk::Ptr<OSPFNode> node,
                                  Fwk::Ptr<const OSPFLSUPacket> pkt);

  /* data members */
  Fwk::Log::Ptr log_;
  PacketFunctor functor_;

  uint32_t router_id_;
  uint32_t area_id_;
  OSPFInterfaceMap::Ptr interfaces_;
  OSPFTopology::Ptr topology_;

  /* operations disallowed */
  OSPFRouter(const OSPFRouter&);
  void operator=(const OSPFRouter&);
};

#endif
