#ifndef DATA_PLANE_H_6H2UAS6L
#define DATA_PLANE_H_6H2UAS6L

#include <string>

#include "arp_cache.h"
#include "fwk/log.h"
#include "fwk/named_interface.h"
#include "fwk/ptr.h"
#include "interface.h"
#include "interface_map.h"
#include "packet.h"
#include "routing_table.h"

/* Forward declarations. */
class ARPPacket;
class ControlPlane;
class EthernetPacket;
class GREPacket;
class ICMPPacket;
class IPPacket;
class UnknownPacket;
struct sr_instance;


class DataPlane : public Fwk::NamedInterface {
 public:
  typedef Fwk::Ptr<const DataPlane> PtrConst;
  typedef Fwk::Ptr<DataPlane> Ptr;

  // Processes incoming packets.
  void packetNew(Packet::Ptr pkt, Interface::PtrConst iface);

  // Sends outgoing packets.
  void outputPacketNew(Fwk::Ptr<EthernetPacket const> pkt,
                       Interface::PtrConst iface);

  InterfaceMap::Ptr interfaceMap() const;

  // Returns the ControlPlane.
  ControlPlane* controlPlane() const { return cp_; }

  // Sets the ControlPlane.
  void controlPlaneIs(ControlPlane* cp) { cp_ = cp; }

  // Returns the router instance.
  struct sr_instance* instance() const { return sr_; }

 protected:
  DataPlane(const std::string& name,
            struct sr_instance* sr,
            RoutingTable::Ptr routing_table,
            ARPCache::Ptr arp_cache);
  virtual ~DataPlane() {}

  /* Operations disallowed. */
  DataPlane(const DataPlane&);
  void operator=(const DataPlane&);

 private:
  class PacketFunctor : public Packet::Functor {
   public:
    PacketFunctor(DataPlane* dp);

    void operator()(ARPPacket*, Interface::PtrConst);
    void operator()(EthernetPacket*, Interface::PtrConst);
    void operator()(GREPacket*, Interface::PtrConst);
    void operator()(ICMPPacket*, Interface::PtrConst);
    void operator()(IPPacket*, Interface::PtrConst);
    void operator()(UnknownPacket*, Interface::PtrConst);

   private:
    DataPlane* dp_;
    Fwk::Log::Ptr log_;
  };

  Fwk::Log::Ptr log_;
  PacketFunctor functor_;
  InterfaceMap::Ptr iface_map_;
  RoutingTable::Ptr routing_table_;
  ARPCache::Ptr arp_cache_;
  ControlPlane* cp_;
  struct sr_instance* sr_;
};

#endif
