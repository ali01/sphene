#include "hw_data_plane.h"

#include <arpa/inet.h>

#include "arp_cache.h"
#include "ethernet_packet.h"
#include "fwk/log.h"
#include "fwk/scoped_lock.h"
#include "ip_packet.h"
#include "ipv4_addr.h"
#include "nf2.h"
#include "nf2util.h"
#include "reg_defines.h"
#include "routing_table.h"
#include "sr_cpu_extension_nf2.h"

struct sr_instance;


HWDataPlane::HWDataPlane(struct sr_instance* sr,
                         RoutingTable::Ptr routing_table,
                         ARPCache::Ptr arp_cache)
    : DataPlane("HWDataPlane", sr, routing_table, arp_cache),
      arp_cache_reactor_(ARPCacheReactor(this)),
      log_(Fwk::Log::LogNew("HWDataPlane")) {
  arp_cache_reactor_.notifierIs(arp_cache);
}


HWDataPlane::ARPCacheReactor::ARPCacheReactor(HWDataPlane* dp)
    : dp_(dp),
      log_(Fwk::Log::LogNew("HWDataPlane::ARPCacheReactor")) { }


void HWDataPlane::ARPCacheReactor::onEntry(ARPCache::Entry::Ptr entry) {
  dp_->writeHWARPCache();
}


void HWDataPlane::ARPCacheReactor::onEntryDel(ARPCache::Entry::Ptr entry) {
  dp_->writeHWARPCache();
}


void HWDataPlane::writeHWARPCache() {
  // Open the NetFPGA for writing registers.
  struct nf2device nf2;
  nf2.device_name = "nf2c0";
  nf2.net_iface = 1;
  if (openDescriptor(&nf2)) {
    perror("openDescriptor()");
    exit(1);
  }

  // Write entries in ARP cache.
  Fwk::ScopedLock<ARPCache> lock(arp_cache_);
  ARPCache::iterator it;
  unsigned int index = 0;
  for (it = arp_cache_->begin(); it != arp_cache_->end(); ++it, ++index) {
    ARPCache::Entry::Ptr entry = it->second;
    EthernetAddr mac = entry->ethernetAddr();
    IPv4Addr ip = entry->ipAddr();
    writeHWARPCacheEntry(&nf2, mac, ip, index);
  }

  // Zero-out remaining entries.
  EthernetAddr zero_mac;
  IPv4Addr zero_ip;
  for (; index < ARPCache::kMaxEntries; ++index)
    writeHWARPCacheEntry(&nf2, zero_mac, zero_ip, index);

  closeDescriptor(&nf2);
}


void HWDataPlane::writeHWARPCacheEntry(struct nf2device* nf2,
                                       const EthernetAddr& mac,
                                       const IPv4Addr& ip,
                                       unsigned int index) {
  // Write MAC address.
  const uint8_t* const mac_addr = mac.data();
  unsigned int mac_hi = 0;
  unsigned int mac_lo = 0;
  mac_hi |= ((unsigned int)mac_addr[0]) << 8;
  mac_hi |= ((unsigned int)mac_addr[1]);
  mac_lo |= ((unsigned int)mac_addr[2]) << 24;
  mac_lo |= ((unsigned int)mac_addr[3]) << 16;
  mac_lo |= ((unsigned int)mac_addr[4]) << 8;
  mac_lo |= ((unsigned int)mac_addr[5]);
  writeReg(nf2, ROUTER_OP_LUT_ARP_TABLE_ENTRY_MAC_HI, mac_hi);
  writeReg(nf2, ROUTER_OP_LUT_ARP_TABLE_ENTRY_MAC_LO, mac_lo);

  // Write IP address.
  const uint32_t ip_addr = ntohl(ip.nbo());
  writeReg(nf2, ROUTER_OP_LUT_ROUTE_TABLE_ENTRY_NEXT_HOP_IP, ip_addr);

  // Set index.
  writeReg(nf2, ROUTER_OP_LUT_ARP_TABLE_WR_ADDR, index);
}
