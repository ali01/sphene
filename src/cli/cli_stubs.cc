#include "cli_stubs.h"

#include <arpa/inet.h>
#include <inttypes.h>
#include <stdio.h>
#include <vector>

#include "fwk/scoped_lock.h"

#include "arp_cache.h"
#include "control_plane.h"
#include "data_plane.h"
#include "interface.h"
#include "interface_map.h"
#include "router.h"
#include "sr_base_internal.h"
#include "routing_table.h"
#include "tunnel.h"
#include "tunnel_map.h"

using std::string;
using std::vector;


int arp_cache_static_entry_add(struct sr_instance* const sr,
                               const uint32_t ip,
                               const uint8_t* const mac) {
  ARPCache::Ptr cache = sr->router->controlPlane()->arpCache();

  ARPCache::Entry::Ptr cache_entry = ARPCache::Entry::New(ntohl(ip), mac);
  cache_entry->typeIs(ARPCache::Entry::kStatic);

  {
    Fwk::ScopedLock<ARPCache> lock(cache);
    cache->entryIs(cache_entry);
  }

  // TODO(ms): Max number of entries.
  return 1;
}


int arp_cache_static_entry_remove(struct sr_instance* const sr,
                                  const uint32_t ip) {
  ARPCache::Ptr cache = sr->router->controlPlane()->arpCache();
  Fwk::ScopedLock<ARPCache> lock(cache);

  ARPCache::Entry::Ptr cache_entry = cache->entry(ntohl(ip));
  if (!cache_entry || cache_entry->type() != ARPCache::Entry::kStatic) {
    return 0;
  }

  cache->entryDel(cache_entry);
  return 1;
}


// Removes all entries from the ARP Cache of the specified type.
static unsigned arp_cache_type_purge(struct sr_instance* const sr,
                                     const ARPCache::Entry::Type type) {
  ARPCache::Ptr cache = sr->router->controlPlane()->arpCache();
  Fwk::ScopedLock<ARPCache> lock(cache);

  // Build list of entries to remove.
  vector<ARPCache::Entry::Ptr> entries;
  for (ARPCache::iterator it = cache->begin(); it != cache->end(); ++it) {
    ARPCache::Entry::Ptr entry = it->second;
    if (entry->type() == type)
      entries.push_back(entry);
  }

  // Remove the chosen entries.
  for (vector<ARPCache::Entry::Ptr>::iterator it = entries.begin();
       it != entries.end(); ++it) {
    cache->entryDel(*it);
  }

  return entries.size();
}


unsigned arp_cache_static_purge(struct sr_instance* const sr) {
  return arp_cache_type_purge(sr, ARPCache::Entry::kStatic);
}


unsigned arp_cache_dynamic_purge(struct sr_instance* const sr) {
  return arp_cache_type_purge(sr, ARPCache::Entry::kDynamic);
}


int router_interface_set_enabled(struct sr_instance* const sr,
                                 const char* const name,
                                 const int enabled) {
  InterfaceMap::Ptr if_map = sr->router->dataPlane()->interfaceMap();
  Fwk::ScopedLock<InterfaceMap> lock(if_map);
  Interface::Ptr iface = if_map->interface(name);

  if (!iface)
    return -1;  // interface does not exist
  if (iface->enabled() == enabled)
    return 1;   // already set to desired value

  iface->enabledIs(enabled);
  return 0;
}


Interface::Ptr router_lookup_interface_via_ip(struct sr_instance* const sr,
                                              const uint32_t ip) {
  InterfaceMap::Ptr if_map = sr->router->dataPlane()->interfaceMap();
  Fwk::ScopedLock<InterfaceMap> lock(if_map);
  return if_map->interfaceAddr(ntohl(ip));
}


Interface::Ptr router_lookup_interface_via_name(struct sr_instance* const sr,
                                                const char* name) {
  InterfaceMap::Ptr if_map = sr->router->dataPlane()->interfaceMap();
  Fwk::ScopedLock<InterfaceMap> lock(if_map);
  return if_map->interface(name);
}


int router_is_interface_enabled(struct sr_instance* const sr,
                                Interface::PtrConst iface) {
  if (!iface)
    return 0;
  return (iface->enabled()) ? 1 : 0;
}


int router_is_ospf_enabled(struct sr_instance* const sr) {
    fprintf( stderr, "not yet implemented: router_is_ospf_enabled\n" );
    return 0;
}


void router_set_ospf_enabled(struct sr_instance* const sr, const int enabled) {
    fprintf( stderr, "not yet implemented: router_set_ospf_enabled\n" );
}


void rtable_route_add(struct sr_instance* const sr,
                      const uint32_t dest,
                      const uint32_t gw,
                      const uint32_t mask,
                      Interface::Ptr iface,
                      const int is_static_route) {
  RoutingTable::Entry::Ptr entry =
    RoutingTable::Entry::New(is_static_route ?
                             RoutingTable::Entry::kStatic :
                             RoutingTable::Entry::kDynamic);
  entry->subnetIs(ntohl(dest), ntohl(mask));
  entry->gatewayIs(ntohl(gw));
  entry->interfaceIs(iface);

  RoutingTable::Ptr rtable = sr->router->controlPlane()->routingTable();
  {
    Fwk::ScopedLock<RoutingTable> lock(rtable);
    rtable->entryIs(entry);
  }

  fprintf(stdout, "Added route: %s/%s gw %s\n",
          string(entry->subnet()).c_str(),
          string(entry->subnetMask()).c_str(),
          string(entry->gateway()).c_str());
}


int rtable_route_remove(struct sr_instance* const sr,
                        const uint32_t dest,
                        const uint32_t mask,
                        const int is_static) {
  RoutingTable::Ptr rtable = sr->router->controlPlane()->routingTable();
  Fwk::ScopedLock<RoutingTable> lock(rtable);

  // TODO(ms): I'm not sure if this matches on type (static/dynamic) correctly.
  //   Specifically, what happens when we learn about a route that exists as
  //   static in the routing table? And vice versa?
  RoutingTable::const_iterator it;
  for (it = rtable->entriesBegin(); it != rtable->entriesEnd(); ++it) {
    RoutingTable::Entry::Ptr entry = it->second;
    if (entry->subnet() == ntohl(dest) &&
        entry->subnetMask() == ntohl(mask) &&
        entry->type() == (is_static ?
                          RoutingTable::Entry::kStatic :
                          RoutingTable::Entry::kDynamic)) {
      rtable->entryDel(entry);

      fprintf(stdout, "Removed route: %s/%s gw %s\n",
              string(entry->subnet()).c_str(),
              string(entry->subnetMask()).c_str(),
              string(entry->gateway()).c_str());

      return 1;
    }
  }

  return 0;  // failed to find given route
}


static void rtable_purge_type(struct sr_instance* const sr,
                              const RoutingTable::Entry::Type type) {
  RoutingTable::Ptr rtable = sr->router->controlPlane()->routingTable();
  Fwk::ScopedLock<RoutingTable> lock(rtable);

  // List of entries to remove.
  vector<RoutingTable::Entry::Ptr> remove;

  // Find candidate entries to remove.
  RoutingTable::Entry::Ptr entry;
  for (RoutingTable::const_iterator it = rtable->entriesBegin();
       it != rtable->entriesEnd(); ++it) {
    entry = it->second;
    if (entry->type() == type)
      remove.push_back(entry);
  }

  // Remove entries.
  vector<RoutingTable::Entry::Ptr>::iterator it;
  for (it = remove.begin(); it != remove.end(); ++it) {
    entry = *it;
    rtable->entryDel(entry);

    fprintf(stdout, "Removed route: %s/%s gw %s\n",
            string(entry->subnet()).c_str(),
            string(entry->subnetMask()).c_str(),
            string(entry->gateway()).c_str());
  }
}


void rtable_purge_all(struct sr_instance* const sr) {
  rtable_purge_type(sr, RoutingTable::Entry::kStatic);
  rtable_purge_type(sr, RoutingTable::Entry::kDynamic);
}


void rtable_purge(struct sr_instance* const sr, const int is_static) {
  rtable_purge_type(sr, (is_static ?
                         RoutingTable::Entry::kStatic :
                         RoutingTable::Entry::kDynamic));
}


int tunnel_add(struct sr_instance* const sr,
               const char* const name,
               const char* const mode,
               const uint32_t dest) {
  InterfaceMap::Ptr if_map = sr->router->dataPlane()->interfaceMap();
  TunnelMap::Ptr tun_map = sr->router->controlPlane()->tunnelMap();
  Fwk::ScopedLock<InterfaceMap> if_map_lock(if_map);
  Fwk::ScopedLock<TunnelMap> tun_map_lock(tun_map);

  // Tunnels share the same namespace as interfaces because they are based on
  // interfaces.
  if (if_map->interface(name))
    return 0;   // interface already exists with name

  // Create a new virtual interface for the tunnel.
  Interface::Ptr iface = Interface::InterfaceNew(name);
  iface->typeIs(Interface::kVirtual);
  iface->enabledIs(true);

  // Create a new tunnel on the interface.
  Tunnel::Ptr tunnel = Tunnel::New(iface);
  tunnel->modeIs(Tunnel::kGRE);  // we only support GRE for now
  tunnel->remoteIs(ntohl(dest));

  // Add the tunnel and virtual interface to their respective maps.
  if_map->interfaceIs(iface);
  tun_map->tunnelIs(tunnel);

  return 1;
}


int tunnel_del(struct sr_instance* const sr, const char* const name) {
  InterfaceMap::Ptr if_map = sr->router->dataPlane()->interfaceMap();
  TunnelMap::Ptr tun_map = sr->router->controlPlane()->tunnelMap();
  Fwk::ScopedLock<InterfaceMap> if_map_lock(if_map);
  Fwk::ScopedLock<TunnelMap> tun_map_lock(tun_map);

  // Make sure we have a tunnel with this name.
  if (!tun_map->tunnel(name))
    return 0;  // no tunnel exists with that name

  // Remove virtual interface and tunnel.
  if_map->interfaceDel(name);
  tun_map->tunnelDel(name);

  return 1;
}


int tunnel_change(struct sr_instance* const sr,
                  const char* const name,
                  const char* const mode,
                  const uint32_t dest) {
  InterfaceMap::Ptr if_map = sr->router->dataPlane()->interfaceMap();
  TunnelMap::Ptr tun_map = sr->router->controlPlane()->tunnelMap();
  Fwk::ScopedLock<InterfaceMap> if_map_lock(if_map);
  Fwk::ScopedLock<TunnelMap> tun_map_lock(tun_map);

  // Make sure we have a tunnel with this name.
  if (!tun_map->tunnel(name))
    return 0;  // no tunnel exists with that name

  Tunnel::Ptr tunnel = tun_map->tunnel(name);

  // Remove tunnel from map first. The tunnel map keys on the remote address,
  // so to ensure consistency, we clear all pointers to the tunnel in the map
  // before updating the tunnel.
  tun_map->tunnelDel(tunnel);

  // Update the attributes of the tunnel.
  tunnel->modeIs(Tunnel::kGRE);  // we only support GRE right now
  tunnel->remoteIs(ntohl(dest));

  // Re-add tunnel to map.
  tun_map->tunnelIs(tunnel);

  return 1;
}
