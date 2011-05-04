#include "hw_data_plane.h"

#include "fwk/log.h"

#include "arp_cache.h"
#include "ethernet_packet.h"
#include "ip_packet.h"
#include "routing_table.h"

struct sr_instance;


HWDataPlane::HWDataPlane(struct sr_instance* sr,
                         RoutingTable::Ptr routing_table,
                         ARPCache::Ptr arp_cache)
    : DataPlane("HWDataPlane", sr, routing_table, arp_cache),
      log_(Fwk::Log::LogNew("HWDataPlane")) { }
