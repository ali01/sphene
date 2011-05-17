#ifndef HW_DATA_PLANE_H_
#define HW_DATA_PLANE_H_

#include "fwk/log.h"
#include "fwk/ptr.h"
#include "fwk/ptr_interface.h"

#include "arp_cache.h"
#include "packet.h"
#include "data_plane.h"
#include "interface.h"
#include "interface_map.h"
#include "routing_table.h"

class ARPPacket;
class EthernetPacket;
class ICMPPacket;
class IPPacket;
struct nf2device;


class HWDataPlane : public DataPlane {
 public:
  typedef Fwk::Ptr<const HWDataPlane> PtrConst;
  typedef Fwk::Ptr<HWDataPlane> Ptr;

  static Ptr New(struct sr_instance* sr,
                 Fwk::Ptr<ARPCache> arp_cache) {
    return new HWDataPlane(sr, arp_cache);
  }

  // Sets the RoutingTable.
  virtual void routingTableIs(RoutingTable::Ptr rtable);

 protected:
  class ARPCacheReactor : public ARPCache::Notifiee {
   public:
    ARPCacheReactor(HWDataPlane* dp);
    virtual void onEntry(ARPCache::Ptr cache, ARPCache::Entry::Ptr entry);
    virtual void onEntryDel(ARPCache::Ptr cache, ARPCache::Entry::Ptr entry);

   protected:
    HWDataPlane* dp_;
    Fwk::Log::Ptr log_;
  };

  class InterfaceReactor : public Interface::Notifiee {
   public:
    InterfaceReactor(HWDataPlane* dp);
    virtual void onIP(Interface::Ptr iface);
    virtual void onMAC(Interface::Ptr iface);
    virtual void onEnabled(Interface::Ptr iface);

   protected:
    HWDataPlane* dp_;
    Fwk::Log::Ptr log_;
  };

  class InterfaceMapReactor : public InterfaceMap::Notifiee {
   public:
    InterfaceMapReactor(HWDataPlane* dp);
    virtual void onInterface(InterfaceMap::Ptr map,
                             Fwk::Ptr<Interface> iface);
    virtual void onInterfaceDel(InterfaceMap::Ptr map,
                                Fwk::Ptr<Interface> iface);

   protected:
    HWDataPlane* dp_;
    Fwk::Log::Ptr log_;
  };

  class RoutingTableReactor : public RoutingTable::Notifiee {
   public:
    RoutingTableReactor(HWDataPlane* dp);
    virtual void onEntry(RoutingTable::Ptr rtable,
                         RoutingTable::Entry::Ptr entry);
    virtual void onEntryDel(RoutingTable::Ptr rtable,
                            RoutingTable::Entry::Ptr entry);

   protected:
    HWDataPlane* dp_;
    Fwk::Log::Ptr log_;
  };

  HWDataPlane(struct sr_instance* sr,
              Fwk::Ptr<ARPCache> arp_cache);

  HWDataPlane(const HWDataPlane&);
  void operator=(const HWDataPlane&);

  // Writes the ARP cache to the hardware.
  void writeHWARPCache();
  void writeHWARPCacheEntry(struct nf2device* nf2,
                            const EthernetAddr& mac,
                            const IPv4Addr& ip,
                            unsigned int index);
  void writeHWIPFilterTable();
  void writeHWIPFilterTableEntry(struct nf2device* nf2,
                                 const IPv4Addr& ip,
                                 unsigned int index);
  void writeHWRoutingTable();
  void writeHWRoutingTableEntry(struct nf2device* nf2,
                                const IPv4Addr& ip,
                                const IPv4Addr& mask,
                                const IPv4Addr& gw,
                                unsigned int encoded_port,
                                unsigned int index);
  void initializeInterface(Fwk::Ptr<Interface> iface);

 private:
  ARPCacheReactor arp_cache_reactor_;
  InterfaceReactor interface_reactor_;
  InterfaceMapReactor interface_map_reactor_;
  RoutingTableReactor routing_table_reactor_;
  Fwk::Log::Ptr log_;
};

#endif
