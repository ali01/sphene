#ifndef OSPF_ROUTER_H_LFORNADU
#define OSPF_ROUTER_H_LFORNADU

#include "fwk/linked_list.h"
#include "fwk/map.h"
#include "fwk/ptr_interface.h"
using Fwk::LinkedList;

#include "ospf_topology.h"
#include "ospf_types.h"
#include "packet.h"

/* Forward declarations. */
class ControlPlane;
class Interface;
class OSPFInterfaceMap;
class OSPFLSUPacket;
class OSPFLink;
class OSPFNode;
class RoutingTable;


class OSPFRouter : public Fwk::PtrInterface<OSPFRouter> {
 public:
  typedef Fwk::Ptr<const OSPFRouter> PtrConst;
  typedef Fwk::Ptr<OSPFRouter> Ptr;

  static const uint16_t kDefaultHelloInterval = 10;

  static Ptr New(const RouterID& router_id, const AreaID& area_id,
                 Fwk::Ptr<RoutingTable> rtable, Fwk::Ptr<ControlPlane> cp) {
    return new OSPFRouter(router_id, area_id, rtable, cp);
  }

  // TODO(ali): perhaps should take OSPFPacket instead of Packet.
  void packetNew(Packet::Ptr pkt, Fwk::Ptr<const Interface> iface);

  const RouterID& routerID() const { return router_id_; }
  const AreaID& areaID() const { return area_id_; }

  Fwk::Ptr<const OSPFInterfaceMap> interfaceMap() const;
  Fwk::Ptr<const OSPFTopology> topology() const;

  Fwk::Ptr<const RoutingTable> routingTable() const;
  Fwk::Ptr<RoutingTable> routingTable();

 protected:
  OSPFRouter(const RouterID& router_id, const AreaID& area_id,
             Fwk::Ptr<RoutingTable> rtable, Fwk::Ptr<ControlPlane> cp);

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

  class NeighborRelationship : public LinkedList<NeighborRelationship>::Node {
   public:
    typedef Fwk::Ptr<const NeighborRelationship> PtrConst;
    typedef Fwk::Ptr<NeighborRelationship> Ptr;

    static Ptr New(Fwk::Ptr<OSPFNode> lsu_sender,
                   Fwk::Ptr<OSPFLink> advertised_neighbor);

    Fwk::Ptr<const OSPFNode> lsuSender() const;
    Fwk::Ptr<OSPFNode> lsuSender();

    Fwk::Ptr<const OSPFLink> advertisedNeighbor() const;
    Fwk::Ptr<OSPFLink> advertisedNeighbor();

   private:
    NeighborRelationship(Fwk::Ptr<OSPFNode> lsu_sender,
                         Fwk::Ptr<OSPFLink> advertised_neighbor);

    /* Data members. */
    Fwk::Ptr<OSPFNode> lsu_sender_;
    Fwk::Ptr<OSPFLink> advertised_neighbor_;

    /* Operations disallowed. */
    NeighborRelationship(const NeighborRelationship&);
    void operator=(const NeighborRelationship&);
  };

  /* Reactor to Topology notifications. */
  class TopologyReactor : public OSPFTopology::Notifiee {
   public:
    typedef Fwk::Ptr<const TopologyReactor> PtrConst;
    typedef Fwk::Ptr<TopologyReactor> Ptr;

    static Ptr New(OSPFRouter::Ptr _r) {
      return new TopologyReactor(_r);
    }

    void onDirtyCleared() { router_->rtable_update(); }

   private:
    TopologyReactor(OSPFRouter::Ptr _r) : router_(_r.ptr()) {}

    /* Data members. */
    OSPFRouter* router_;

    /* Operations disallowed. */
    TopologyReactor(const TopologyReactor&);
    void operator=(const TopologyReactor&);
  };


  /* -- OSPFRouter private member functions. -- */

  /* Uses the optimal spanning tree computed by OSPFTopology to update all
     entries in the routing table. */
  void rtable_update();

  /* Adds the subnets connected to DEST to the routing table via NEXT_HOP.
     Assumes that routing_table_ is already locked. */
  void rtable_add_dest(OSPFNode::PtrConst next_hop, OSPFNode::PtrConst dest);

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
                                  Fwk::Ptr<const OSPFLSUPacket> pkt);

  /* Sends the given LSU packet to all neighbors except the packet's original
     sender. */
  void forward_flood_lsu_packet(Fwk::Ptr<OSPFLSUPacket> pkt) const;

  /* Sends PKT to the node in the topology with DEST_ID. */
  void forward_pkt_to_neighbor(const RouterID& neighbor_id,
                            Fwk::Ptr<OSPFPacket> pkt) const;

  /* Obtains the staged NeighborRelationship object with the specified
     LSU_SENDER_ID and ADVERTISED_NEIGHBOR_ID from the LINKS_STAGED
     multimap if it exists. A NeighborRelationship object will exist if the
     node with ID of LSU_SENDER_ID has previously advertised that it is
     connected to the node with ID ADVERTISED_NEIGHBOR_ID. If no such
     NeighborRelationship object exists in the multimap, this function returns
     NULL. */
  NeighborRelationship::PtrConst staged_nbr(const RouterID& lsu_sender_id,
                                            const RouterID& adv_nb_id) const;
  NeighborRelationship::Ptr staged_nbr(const RouterID& lsu_sender_id,
                                       const RouterID& adv_nb_id);

  /* Adds the specified NeighborRelationship object to the LINKS_STAGED
     multimap. Returns false if NBR is NULL or if it already exists in
     the multimap, true otherwise. */
  bool stage_nbr(NeighborRelationship::Ptr nbr);

  /* Commits the specified NeighborRelationship object to the router's
     network topology and removes it from the LINKS_STAGED multimap.
     This function makes use of unstage_nbr(). */
  void commit_nbr(NeighborRelationship::Ptr nbr);

  /* Removes the specified NeighborRelationship object from the LINKS_STAGED
     multimap. Returns false if NBR is NULL or if it doesn't exist in the
     multimap, true otherwise. */
  bool unstage_nbr(NeighborRelationship::Ptr nbr);

  /* -- OSPFRouter data members. -- */

  PacketFunctor functor_;

  const RouterID router_id_;
  const AreaID area_id_;
  Fwk::Ptr<OSPFNode> router_node_;
  Fwk::Ptr<OSPFInterfaceMap> interfaces_;
  Fwk::Ptr<OSPFTopology> topology_;
  TopologyReactor::Ptr topology_reactor_; /* Reactor: Topology notifications */

  /* External references. */
  Fwk::Ptr<RoutingTable> routing_table_;
  ControlPlane* control_plane_; /* Weak pointer prevents circular references */

  /* Logical Multimap<LSUSenderRouterID,NeighborRelationshipList>: Keeps track
     of neighbor relationships that have not yet been commited to the topology.
     This is necessary to avoid corrupting the topology in response to
     contradicting link-state advertisements. Refer to the PWOSPF specification
     for more details. */
  Fwk::Map<RouterID,LinkedList<NeighborRelationship> > links_staged_;

  /* operations disallowed */
  OSPFRouter(const OSPFRouter&);
  void operator=(const OSPFRouter&);
};

#endif
