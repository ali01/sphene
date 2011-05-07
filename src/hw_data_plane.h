#ifndef HW_DATA_PLANE_H_
#define HW_DATA_PLANE_H_

#include "fwk/log.h"
#include "fwk/ptr.h"
#include "fwk/ptr_interface.h"

#include "arp_cache.h"
#include "packet.h"
#include "data_plane.h"
#include "interface_map.h"

class ARPPacket;
class EthernetPacket;
class ICMPPacket;
class Interface;
class IPPacket;
struct nf2device;


class HWDataPlane : public DataPlane {
 public:
  typedef Fwk::Ptr<const HWDataPlane> PtrConst;
  typedef Fwk::Ptr<HWDataPlane> Ptr;

  static Ptr New(struct sr_instance* sr,
                 Fwk::Ptr<RoutingTable> routing_table,
                 Fwk::Ptr<ARPCache> arp_cache) {
    return new HWDataPlane(sr, routing_table, arp_cache);
  }

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

  HWDataPlane(struct sr_instance* sr,
              Fwk::Ptr<RoutingTable> routing_table,
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
  void initializeInterface(Fwk::Ptr<Interface> iface);

 private:
  ARPCacheReactor arp_cache_reactor_;
  InterfaceMapReactor interface_map_reactor_;
  Fwk::Log::Ptr log_;
};

#endif
