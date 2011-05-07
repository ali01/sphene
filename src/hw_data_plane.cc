#include "hw_data_plane.h"

#include <arpa/inet.h>
#include <cstdio>
#include <cstring>
#include <unistd.h>

#include "arp_cache.h"
#include "ethernet_packet.h"
#include "fwk/log.h"
#include "fwk/scoped_lock.h"
#include "interface.h"
#include "interface_map.h"
#include "ip_packet.h"
#include "ipv4_addr.h"
#include "nf2.h"
#include "nf2util.h"
#include "reg_defines.h"
#include "routing_table.h"
#include "sr_cpu_extension_nf2.h"

struct sr_instance;
static const char* const kDefaultIfaceName = "nf2c0";


HWDataPlane::HWDataPlane(struct sr_instance* sr,
                         RoutingTable::Ptr routing_table,
                         ARPCache::Ptr arp_cache)
    : DataPlane("HWDataPlane", sr, routing_table, arp_cache),
      arp_cache_reactor_(ARPCacheReactor(this)),
      interface_map_reactor_(InterfaceMapReactor(this)),
      log_(Fwk::Log::LogNew("HWDataPlane")) {
  arp_cache_reactor_.notifierIs(arp_cache);
  interface_map_reactor_.notifierIs(iface_map_);

  // Reset the hardware.
  struct nf2device nf2;
  nf2.device_name = kDefaultIfaceName;
  nf2.net_iface = 1;
  if (openDescriptor(&nf2)) {
    perror("openDescriptor()");
    exit(1);
  }
  writeReg(&nf2, CPCI_REG_CTRL, 0x00010100);
  closeDescriptor(&nf2);
  usleep(2000);
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


HWDataPlane::InterfaceMapReactor::InterfaceMapReactor(HWDataPlane* dp)
    : dp_(dp),
      log_(Fwk::Log::LogNew("HWDataPlane::InterfaceMapReactor")) { }


void HWDataPlane::InterfaceMapReactor::onInterface(Interface::Ptr iface) {
  dp_->initializeInterface(iface);
  dp_->writeHWIPFilterTable();
}


void HWDataPlane::InterfaceMapReactor::onInterfaceDel(Interface::Ptr iface) {
  dp_->writeHWIPFilterTable();
}


void HWDataPlane::writeHWARPCache() {
  // Open the NetFPGA for writing registers.
  struct nf2device nf2;
  nf2.device_name = kDefaultIfaceName;
  nf2.net_iface = 1;
  if (openDescriptor(&nf2)) {
    perror("openDescriptor()");
    exit(1);
  }

  // Write entries in ARP cache.
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


void HWDataPlane::writeHWIPFilterTable() {
  // Open the NetFPGA for writing registers.
  struct nf2device nf2;
  nf2.device_name = kDefaultIfaceName;
  nf2.net_iface = 1;
  if (openDescriptor(&nf2)) {
    perror("openDescriptor()");
    exit(1);
  }

  // InterfaceMap is not locked here because we are called by a
  // notification. We assume the InterfaceMap was already locked.
  InterfaceMap::iterator it;
  unsigned int index = 0;
  for (it = iface_map_->begin(); it != iface_map_->end(); ++it, ++index) {
    Interface::Ptr iface = it->second;
    writeHWIPFilterTableEntry(&nf2, iface->ip(), index);
  }

  // Zero-out remaining entries.
  IPv4Addr zero_ip;
  for (; index < InterfaceMap::kMaxInterfaces; ++index)
    writeHWIPFilterTableEntry(&nf2, zero_ip, index);

  closeDescriptor(&nf2);
}


void HWDataPlane::writeHWIPFilterTableEntry(struct nf2device* nf2,
                                            const IPv4Addr& ip,
                                            unsigned int index) {
  uint32_t ip_addr = ntohl(ip.nbo());  // little endian
  writeReg(nf2, ROUTER_OP_LUT_DST_IP_FILTER_TABLE_ENTRY_IP, ip_addr);
  writeReg(nf2, ROUTER_OP_LUT_DST_IP_FILTER_TABLE_WR_ADDR, index);
}


void HWDataPlane::initializeInterface(Interface::Ptr iface) {
  // Only initialize hardware interfaces.
  if (iface->type() != Interface::kHardware)
    return;
  // Skip already-initialized interfaces.
  if (iface->socketDescriptor() >= 0)
    return;

  // Assume we are called as the interface map is growing.
  unsigned int index = iface_map_->interfaces() - 1;

  // Open the NetFPGA for writing registers.
  struct nf2device nf2;
  nf2.device_name = kDefaultIfaceName;
  nf2.net_iface = 1;
  if (openDescriptor(&nf2)) {
    perror("openDescriptor()");
    exit(1);
  }

  // Write the MAC address of the interface.
  const uint8_t* mac_addr = iface->mac().data();
  unsigned int mac_hi = 0;
  unsigned int mac_lo = 0;
  mac_hi |= ((unsigned int)mac_addr[0]) << 8;
  mac_hi |= ((unsigned int)mac_addr[1]);
  mac_lo |= ((unsigned int)mac_addr[2]) << 24;
  mac_lo |= ((unsigned int)mac_addr[3]) << 16;
  mac_lo |= ((unsigned int)mac_addr[4]) << 8;
  mac_lo |= ((unsigned int)mac_addr[5]);
  writeReg(&nf2, ROUTER_OP_LUT_MAC_0_HI + (index * 0x8), mac_hi);
  writeReg(&nf2, ROUTER_OP_LUT_MAC_0_LO + (index * 0x8), mac_lo);

  closeDescriptor(&nf2);
}
