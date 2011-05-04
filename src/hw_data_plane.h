#ifndef HW_DATA_PLANE_H_
#define HW_DATA_PLANE_H_

#include "fwk/log.h"
#include "fwk/ptr.h"
#include "fwk/ptr_interface.h"

#include "arp_cache.h"
#include "packet.h"
#include "data_plane.h"

class ARPPacket;
class EthernetPacket;
class ICMPPacket;
class Interface;
class IPPacket;


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
  HWDataPlane(struct sr_instance* sr,
              Fwk::Ptr<RoutingTable> routing_table,
              Fwk::Ptr<ARPCache> arp_cache);

  HWDataPlane(const HWDataPlane&);
  void operator=(const HWDataPlane&);

 private:
  Fwk::Log::Ptr log_;
};

#endif
