#ifndef OSPF_ROUTER_H_LFORNADU
#define OSPF_ROUTER_H_LFORNADU

#include "fwk/map.h"
#include "fwk/ptr_interface.h"

#include "ospf_adv_map.h"
#include "ospf_interface_map.h"
#include "ospf_topology.h"
#include "ospf_types.h"
#include "packet.h"
#include "routing_table.h"

/* Forward declarations. */
class ControlPlane;
class Interface;
class InterfaceMap;
class OSPFAdvertisementSet;
class OSPFInterface;
class OSPFLSUPacket;
class OSPFLink;
class OSPFPacket;
class OSPFNode;


class OSPFRouter : public Fwk::PtrInterface<OSPFRouter> {
 public:
  typedef Fwk::Ptr<const OSPFRouter> PtrConst;
  typedef Fwk::Ptr<OSPFRouter> Ptr;

  static Ptr New(const RouterID& router_id,
                 const AreaID& area_id,
                 Fwk::Ptr<RoutingTable> rtable,
                 Fwk::Ptr<InterfaceMap> iface_map,
                 ControlPlane* cp) {
    return new OSPFRouter(router_id, area_id, rtable, iface_map, cp);
  }

  class Notifiee : public Fwk::PtrInterface<Notifiee> {
   public:
    typedef Fwk::Ptr<const Notifiee> PtrConst;
    typedef Fwk::Ptr<Notifiee> Ptr;

    /* Notifications supported. */
    virtual void onLinkStateFlood(OSPFRouter::Ptr _r) = 0;

   protected:
    Notifiee() {}
    virtual ~Notifiee() {}

   private:
    /* Operations disallowed. */
    Notifiee(const Notifiee&);
    void operator=(const Notifiee&);
  };

  void packetNew(Fwk::Ptr<OSPFPacket> pkt, Fwk::Ptr<const Interface> iface);

  /* Accessors. */

  const RouterID& routerID() const { return router_node_->routerID(); }
  const AreaID& areaID() const { return area_id_; }

  Fwk::Ptr<const OSPFInterfaceMap> interfaceMap() const;
  Fwk::Ptr<OSPFInterfaceMap> interfaceMap();

  Fwk::Ptr<const OSPFTopology> topology() const;
  Fwk::Ptr<OSPFTopology> topology();

  Fwk::Ptr<const RoutingTable> routingTable() const;
  Fwk::Ptr<RoutingTable> routingTable();

  Notifiee::PtrConst notifiee() const { return notifiee_; }
  Notifiee::Ptr notifiee() { return notifiee_; }

  /* Mutators. */

  void routerIDIs(const RouterID& id) { router_node_->routerIDIs(id); }
  void notifieeIs(Notifiee::Ptr _n) { notifiee_ = _n; }

  /* Signals. */

  /* Signal to indicate that the LSU interval has elapsed. */
  void onLinkStateInterval() { flood_lsu(); }

  /* Signal: connectivity to direct neighbors may have changed*/
  void onLinkStateUpdate();

 protected:
  OSPFRouter(const RouterID& router_id,
             const AreaID& area_id,
             Fwk::Ptr<RoutingTable> rtable,
             Fwk::Ptr<InterfaceMap> iface_map,
             ControlPlane* cp);

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
  };

  /* Reactor to Topology notifications. */
  class TopologyReactor : public OSPFTopology::Notifiee {
   public:
    typedef Fwk::Ptr<const TopologyReactor> PtrConst;
    typedef Fwk::Ptr<TopologyReactor> Ptr;

    static Ptr New(OSPFRouter* _r) {
      return new TopologyReactor(_r);
    }

    void onDirtyCleared() { router_->rtable_update(); }

   private:
    TopologyReactor(OSPFRouter* _r) : router_(_r) {}

    /* Data members. */
    OSPFRouter* router_;

    /* Operations disallowed. */
    TopologyReactor(const TopologyReactor&);
    void operator=(const TopologyReactor&);
  };

  class OSPFInterfaceMapReactor : public OSPFInterfaceMap::Notifiee {
   public:
    typedef Fwk::Ptr<const OSPFInterfaceMapReactor> PtrConst;
    typedef Fwk::Ptr<OSPFInterfaceMapReactor> Ptr;

    static Ptr New(OSPFRouter* _r) {
      return new OSPFInterfaceMapReactor(_r);
    }

    void onInterface(OSPFInterfaceMap::Ptr _im, OSPFInterface::Ptr iface);
    void onInterfaceDel(OSPFInterfaceMap::Ptr, OSPFInterface::Ptr);
    void onGateway(OSPFInterfaceMap::Ptr, OSPFInterface::Ptr, OSPFGateway::Ptr);
    void onGatewayDel(OSPFInterfaceMap::Ptr, OSPFInterface::Ptr,
                      OSPFGateway::Ptr);

   private:
    OSPFInterfaceMapReactor(OSPFRouter* _r) : ospf_router_(_r) {}

    /* Data members. */
    OSPFRouter* ospf_router_;

    /* Operations disallowed. */
    OSPFInterfaceMapReactor(const OSPFInterfaceMapReactor&);
    void operator=(const OSPFInterfaceMapReactor&);
  };

  class RoutingTableReactor : public RoutingTable::Notifiee {
   public:
    typedef Fwk::Ptr<const RoutingTableReactor> PtrConst;
    typedef Fwk::Ptr<RoutingTableReactor> Ptr;

    static Ptr New(OSPFRouter* _r) {
      return new RoutingTableReactor(_r);
    }

    void onEntry(RoutingTable::Ptr rtable,
                 RoutingTable::Entry::Ptr entry);

    void onEntryDel(RoutingTable::Ptr rtable,
                    RoutingTable::Entry::Ptr entry);

   private:
    RoutingTableReactor(OSPFRouter* _r) : ospf_router_(_r) {}

    /* Data members. */
    OSPFRouter* ospf_router_;

    /* Operations disallowed. */
    RoutingTableReactor(const RoutingTableReactor&);
    void operator=(const RoutingTableReactor&);
  };

  /* -- OSPFRouter private member functions. -- */

  void outputPacketNew(Fwk::Ptr<OSPFPacket> ospf_pkt);

  /* Uses the optimal spanning tree computed by OSPFTopology to update all
     entries in the routing table. */
  void rtable_update();

  /* Adds the subnets connected to DEST to the routing table via NEXT_HOP.
     Assumes that routing_table_ is already locked. */
  void rtable_add_dest(OSPFNode::PtrConst next_hop, OSPFNode::PtrConst dest);

  /* Adds an entry for the given gateway to the routing table.
     Assumes that routing_table_ is already locked. */
  void rtable_add_gateway(const IPv4Addr& subnet,
                          const IPv4Addr& mask,
                          const IPv4Addr& gateway,
                          OSPFInterface::PtrConst iface);

  /* Processes the LSU advertisements enclosed in PKT, an OSPFLSUPacket
     received from SENDER. This function establishes a bi-directional link in
     the router's network topology to each advertised neighbor, N_i, only if N_i
     has also advertised connectivity to SENDER. In the case that N_i has not
     previously advertised connectivity to SENDER, this function stages a
     NeighborRelationship object to the LINKS_STAGED multimap to be committed
     whenever said advertisement is received.

     This function assumes that SENDER is already in the router's network
     topology.
  */
  void process_lsu_advertisements(Fwk::Ptr<OSPFNode> sender,
                                  Fwk::Ptr<const OSPFLSUPacket> pkt,
                                  Interface::PtrConst iface);

  /* Removes active links connected to SENDER if they are not confirmed in
     the given OSPF LSU packet. */
  void remove_unconfirmed_links(OSPFNode::Ptr sender,
                                Fwk::Ptr<OSPFAdvertisementSet> confirmed_advs);

  /* Forwards OSPF PKT to the node in the topology with DEST_ID. */
  void forward_packet_to_gateway(Fwk::Ptr<OSPFPacket> pkt,
                                 Fwk::Ptr<OSPFGateway> gw_obj) const;

  /* Sends the given LSU packet to all neighbors except the packet's original
     sender. */
  void forward_lsu_flood(Fwk::Ptr<OSPFLSUPacket> pkt) const;

  /* Sends a new LSU update to all connected neighbors. */
  void flood_lsu();

  /* Sends a new LSU update to all neighbors directly connected to IFACE. */
  void flood_lsu_out_interface(Fwk::Ptr<OSPFInterface> iface);

  /* Constructs a full link-state update packet destined to neighbor with id
     NBR_ID connected at interface IFACE. */
  Fwk::Ptr<OSPFLSUPacket> build_lsu_to_gateway(Fwk::Ptr<OSPFInterface>,
                                               Fwk::Ptr<OSPFGateway>) const;

  /* Sets link state advertisements in the provided OSPF packet from the
     given interface starting with the advertisement with index STARTING_IX. */
  static void set_lsu_adv_from_interface(Fwk::Ptr<OSPFLSUPacket>,
                                         Fwk::Ptr<OSPFInterface>,
                                         uint32_t& starting_ix);

  /* If one doesn't already exist, this function creates an OSPF gateway from
     the given routing table entry. */
  void update_iface_from_rtable_entry_new(OSPFInterface::Ptr,
                                          RoutingTable::Entry::PtrConst);

  void update_iface_from_rtable_entry_delete(OSPFInterface::Ptr,
                                             RoutingTable::Entry::PtrConst);

  /* -- OSPFRouter data members. -- */

  const AreaID area_id_;
  Fwk::Ptr<OSPFNode> router_node_;
  Fwk::Ptr<OSPFInterfaceMap> interfaces_;
  Fwk::Ptr<OSPFTopology> topology_;
  TopologyReactor::Ptr topology_reactor_; /* Reactor: Topology notifications */
  OSPFInterfaceMapReactor::Ptr im_reactor_;
  RoutingTableReactor::Ptr rtable_reactor_;

  uint32_t lsu_seqno_;
  bool lsu_dirty_; /* Links to neighbors have changed since last LSU flood */

  /* Singleton notifiee. */
  Notifiee::Ptr notifiee_;

  /* External references. */
  Fwk::Ptr<RoutingTable> routing_table_;
  ControlPlane* control_plane_; /* Weak pointer prevents circular references */

  /* Must be initialized last because constructor gains references to
     several data members above through the this ptr of OSPFRouter. */
  PacketFunctor functor_;

  /* Keeps track of neighbor relationships that have not yet been commited to
     the topology. This is necessary to avoid corrupting the topology in
     response to contradicting link-state advertisements. Refer to the PWOSPF
     specification for more details. */
  OSPFAdvertisementMap advs_staged_;

  /* operations disallowed */
  OSPFRouter(const OSPFRouter&);
  void operator=(const OSPFRouter&);
};

#endif
