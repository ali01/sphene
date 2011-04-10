#ifndef CONTROL_PLANE_H_
#define CONTROL_PLANE_H_

#include <string>

#include "arp_cache.h"
#include "data_plane.h"
#include "fwk/log.h"
#include "fwk/named_interface.h"
#include "fwk/ptr.h"
#include "interface.h"
#include "interface_map.h"
#include "packet.h"
#include "routing_table.h"

// Forward declarations.
class ARPPacket;
class EthernetPacket;
class ICMPPacket;
class IPPacket;
class UnknownPacket;


class ControlPlane : public Fwk::NamedInterface {
 public:
  typedef Fwk::Ptr<const ControlPlane> PtrConst;
  typedef Fwk::Ptr<ControlPlane> Ptr;

  static Ptr ControlPlaneNew() {
    return new ControlPlane("ControlPlane");
  }

  void packetNew(Fwk::Ptr<EthernetPacket> pkt, Interface::PtrConst iface);

  // Returns the DataPlane.
  DataPlane::Ptr dataPlane() const { return dp_; }

  // Sets the DataPlane.
  void dataPlaneIs(DataPlane::Ptr dp) { dp_ = dp; }

  ARPCache::Ptr arpCache() const { return arp_cache_; }

  RoutingTable::Ptr routingTable() const { return routing_table_; }

 protected:
  ControlPlane(const std::string& name);
  virtual ~ControlPlane() { }

 private:
  class PacketFunctor : public Packet::Functor {
   public:
    PacketFunctor(ControlPlane* cp);

    void operator()(ARPPacket*, Interface::PtrConst);
    void operator()(EthernetPacket*, Interface::PtrConst);
    void operator()(ICMPPacket*, Interface::PtrConst);
    void operator()(IPPacket*, Interface::PtrConst);
    void operator()(UnknownPacket*, Interface::PtrConst);

   private:
    ControlPlane* cp_;
    Fwk::Log::Ptr log_;
  };

  // Operations disallowed.
  ControlPlane(const ControlPlane&);
  void operator=(const ControlPlane&);

  Fwk::Log::Ptr log_;
  PacketFunctor functor_;
  ARPCache::Ptr arp_cache_;
  RoutingTable::Ptr routing_table_;
  DataPlane::Ptr dp_;
};

#endif
