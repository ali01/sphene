#ifndef OSPF_ROUTER_H_LFORNADU
#define OSPF_ROUTER_H_LFORNADU

#include "fwk/linked_list.h"
#include "fwk/log.h"
#include "fwk/map.h"
#include "fwk/ptr_interface.h"
using Fwk::LinkedList;

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
  /* Nested double-dispatch Packet::Functor. */
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

  /* Allows the insertion of Node objects into Fwk::LinkedList. */
  class NodeWrapper : public LinkedList<NodeWrapper>::Node {
   public:
    typedef Fwk::Ptr<const NodeWrapper> PtrConst;
    typedef Fwk::Ptr<NodeWrapper> Ptr;

    static Ptr New(Fwk::Ptr<OSPFNode> node);

    Fwk::Ptr<const OSPFNode> node() const;
    Fwk::Ptr<OSPFNode> node();

   private:
    NodeWrapper(Fwk::Ptr<OSPFNode> node);

    /* Data members. */
    Fwk::Ptr<OSPFNode> node_;

    /* Operations disallowed. */
    NodeWrapper(const NodeWrapper&);
    void operator=(const NodeWrapper&);
  };


  /* OSPFRouter private member functions. */
  void process_lsu_advertisements(Fwk::Ptr<OSPFNode> sender,
                                  Fwk::Ptr<const OSPFLSUPacket> pkt);

  /* OSPfRouter data members. */
  Fwk::Log::Ptr log_;
  PacketFunctor functor_;

  RouterID router_id_;
  AreaID area_id_;
  Fwk::Ptr<OSPFNode> router_node_;
  Fwk::Ptr<OSPFInterfaceMap> interfaces_;
  Fwk::Ptr<OSPFTopology> topology_;
  Fwk::Ptr<RoutingTable> routing_table_;

  /* Logical Multimap<RouterID,AdvertisedNeighbors>: Keeps track of neighbor
     relationships that have not yet been commited to the topology. This is
     necessary to avoid corrupting the topology in response to contradicting
     link-state advertisements. Refer to the PWOSPF specification for more
     details. */
  Fwk::Map<RouterID,LinkedList<NodeWrapper> > links_staged_;

  /* operations disallowed */
  OSPFRouter(const OSPFRouter&);
  void operator=(const OSPFRouter&);
};

#endif
