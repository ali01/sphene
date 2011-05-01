#ifndef OSPF_ROUTER_H_LFORNADU
#define OSPF_ROUTER_H_LFORNADU

#include "fwk/log.h"
#include "fwk/map.h"
#include "fwk/ptr_interface.h"

#include "ospf_types.h"
#include "packet.h"

/* Forward declarations. */
class Interface;
class OSPFInterfaceMap;
class OSPFLSUPacket;
class OSPFNode;
class OSPFTopology;
class RoutingTable;


class OSPFRouter : public Fwk::PtrInterface<OSPFRouter> {
 public:
  typedef Fwk::Ptr<const OSPFRouter> PtrConst;
  typedef Fwk::Ptr<OSPFRouter> Ptr;

  static const uint16_t kDefaultHelloInterval = 10;

  static Ptr New(const RouterID& router_id, const AreaID& area_id) {
    return new OSPFRouter(router_id, area_id);
  }

  // TODO(ali): perhaps should take OSPFPacket instead of Packet.
  void packetNew(Packet::Ptr pkt, Fwk::Ptr<const Interface> iface);

  const RouterID& routerID() const { return router_id_; }
  const AreaID& areaID() const { return area_id_; }

  Fwk::Ptr<const OSPFInterfaceMap> interfaceMap() const;
  Fwk::Ptr<const OSPFTopology> topology() const;

  Fwk::Ptr<const RoutingTable> routingTable() const;
  Fwk::Ptr<RoutingTable> routingTable();

  void routingTableIs(Fwk::Ptr<RoutingTable> rtable);

 protected:
  OSPFRouter(const RouterID& router_id, const AreaID& area_id);

 private:
  class PacketFunctor : public Packet::Functor {
   public:
    PacketFunctor(OSPFRouter* ospf_router);

    void operator()(OSPFPacket*, Fwk::Ptr<const Interface>);
    void operator()(OSPFHelloPacket*, Fwk::Ptr<const Interface>);
    void operator()(OSPFLSUPacket*, Fwk::Ptr<const Interface>);

   private:
    OSPFRouter* ospf_router_;
    OSPFNode* router_node_;
    OSPFInterfaceMap* interfaces_;
    OSPFTopology* topology_;
    Fwk::Log::Ptr log_;
  };

  void process_lsu_advertisements(Fwk::Ptr<OSPFNode> node,
                                  Fwk::Ptr<const OSPFLSUPacket> pkt);

  /* data members */
  Fwk::Log::Ptr log_;
  PacketFunctor functor_;

  RouterID router_id_;
  AreaID area_id_;
  Fwk::Ptr<OSPFNode> router_node_;
  Fwk::Ptr<OSPFInterfaceMap> interfaces_;
  Fwk::Ptr<OSPFTopology> topology_;
  Fwk::Ptr<RoutingTable> routing_table_;

  /* operations disallowed */
  OSPFRouter(const OSPFRouter&);
  void operator=(const OSPFRouter&);
};

#endif
