#include "hw_data_plane.h"

#include <algorithm>
#include <arpa/inet.h>
#include <cstdio>
#include <cstring>
#include <linux/netdevice.h>
#include <linux/sockios.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>

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
#ifdef REF_REG_DEFINES
#include "reg_defines.h"
#else
#include "custom_reg_defines.h"
#endif
#include "routing_table.h"
#include "sr_cpu_extension_nf2.h"

using std::vector;

struct sr_instance;
static const char* const kDefaultIfaceName = "nf2c0";
static const size_t kMaxHWRoutingTableEntries = 32;


HWDataPlane::HWDataPlane(struct sr_instance* sr,
                         RoutingTable::Ptr routing_table,
                         ARPCache::Ptr arp_cache)
    : DataPlane("HWDataPlane", sr, routing_table, arp_cache),
      arp_cache_reactor_(this),
      interface_reactor_(this),
      interface_map_reactor_(this),
      routing_table_reactor_(this),
      log_(Fwk::Log::LogNew("HWDataPlane")) {
  arp_cache_reactor_.notifierIs(arp_cache);
  interface_map_reactor_.notifierIs(iface_map_);
  routing_table_reactor_.notifierIs(routing_table);

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


void HWDataPlane::ARPCacheReactor::onEntry(ARPCache::Ptr cache,
                                           ARPCache::Entry::Ptr entry) {
  dp_->writeHWARPCache();
}


void HWDataPlane::ARPCacheReactor::onEntryDel(ARPCache::Ptr cache,
                                              ARPCache::Entry::Ptr entry) {
  dp_->writeHWARPCache();
}


HWDataPlane::InterfaceReactor::InterfaceReactor(HWDataPlane* dp)
    : dp_(dp),
      log_(Fwk::Log::LogNew("HWDataPlane::InterfaceReactor")) { }


void HWDataPlane::InterfaceReactor::onIP(Interface::Ptr iface) {
  DLOG << "IP address on " << iface->name() << " changed to " << iface->ip();
  dp_->writeHWIPFilterTable();
}


void HWDataPlane::InterfaceReactor::onMAC(Interface::Ptr iface) {
  // TODO(ms): Implement.
}


HWDataPlane::InterfaceMapReactor::InterfaceMapReactor(HWDataPlane* dp)
    : dp_(dp),
      log_(Fwk::Log::LogNew("HWDataPlane::InterfaceMapReactor")) { }


void HWDataPlane::InterfaceMapReactor::onInterface(InterfaceMap::Ptr map,
                                                   Interface::Ptr iface) {
  dp_->initializeInterface(iface);
  dp_->writeHWIPFilterTable();

  // Register to get notifications from the interface itself.
  dp_->interface_reactor_.notifierIs(iface);
}


void HWDataPlane::InterfaceMapReactor::onInterfaceDel(InterfaceMap::Ptr map,
                                                      Interface::Ptr iface) {
  dp_->writeHWIPFilterTable();
}


HWDataPlane::RoutingTableReactor::RoutingTableReactor(HWDataPlane* dp)
    : dp_(dp),
      log_(Fwk::Log::LogNew("HWDataPlane::RoutingTableReactor")) { }


void HWDataPlane::RoutingTableReactor::onEntry(
    RoutingTable::Ptr rtable, RoutingTable::Entry::Ptr entry) {
  dp_->writeHWRoutingTable();
}


void HWDataPlane::RoutingTableReactor::onEntryDel(
    RoutingTable::Ptr rtable, RoutingTable::Entry::Ptr entry) {
  dp_->writeHWRoutingTable();
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
  writeReg(nf2, ROUTER_OP_LUT_ARP_TABLE_ENTRY_MAC_HI_REG, mac_hi);
  writeReg(nf2, ROUTER_OP_LUT_ARP_TABLE_ENTRY_MAC_LO_REG, mac_lo);

  // Write IP address.
  const uint32_t ip_addr = ntohl(ip.nbo());
  writeReg(nf2, ROUTER_OP_LUT_ROUTE_TABLE_ENTRY_NEXT_HOP_IP_REG, ip_addr);

  // Set index.
  writeReg(nf2, ROUTER_OP_LUT_ARP_TABLE_WR_ADDR_REG, index);
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
  writeReg(nf2, ROUTER_OP_LUT_DST_IP_FILTER_TABLE_ENTRY_IP_REG, ip_addr);
  writeReg(nf2, ROUTER_OP_LUT_DST_IP_FILTER_TABLE_WR_ADDR_REG, index);
}


namespace {

// Comparison function for std::sort(). Sorts routing table entries, longest
// subnet mask first.
bool lpmSorter(RoutingTable::Entry::Ptr a,
               RoutingTable::Entry::Ptr b) {
  return (a->subnetMask() > b->subnetMask());
}

}  // namespace


void HWDataPlane::writeHWRoutingTable() {
  // Get all routing table entries into a vector for sorting.
  vector<RoutingTable::Entry::Ptr> entries;
  for (RoutingTable::iterator it = routing_table_->entriesBegin();
       it != routing_table_->entriesEnd();
       ++it) {
    // TODO(ms): Figure out how to include routes over virtual interfaces.
    if (it->second->interface()->type() == Interface::kVirtual)
      continue;

    entries.push_back(it->second);
  }

  // Sort entries in LPM-first order.
  std::sort(entries.begin(), entries.end(), lpmSorter);

  // Open the NetFPGA for writing registers.
  struct nf2device nf2;
  nf2.device_name = kDefaultIfaceName;
  nf2.net_iface = 1;
  if (openDescriptor(&nf2)) {
    perror("openDescriptor()");
    exit(1);
  }

  // Write the routing table entries, LPM first.
  unsigned int index;
  for (index = 0; index < entries.size(); ++index) {
    if (index >= kMaxHWRoutingTableEntries) {
      WLOG << "Routing table is too large to fit entirely in hardware";
      break;
    }

    RoutingTable::Entry::Ptr entry = entries[index];
    Interface::PtrConst iface = entry->interface();

    // The port number is calculated in "one-hot-encoded format":
    //   iface num    port
    //   0            1
    //   1            4
    //   2            16
    //   3            64
    // For iface num i, this is 4**i.
    unsigned int encoded_port = (1 << (iface->index() * 2));

    writeHWRoutingTableEntry(&nf2,
                             entry->subnet(),
                             entry->subnetMask(),
                             entry->gateway(),
                             encoded_port,
                             index);
  }

  // Zero-out remaining entries.
  IPv4Addr zero_ip;
  for (; index < kMaxHWRoutingTableEntries; ++index)
    writeHWRoutingTableEntry(&nf2, zero_ip, zero_ip, zero_ip, 0, index);

  closeDescriptor(&nf2);
}


void HWDataPlane::writeHWRoutingTableEntry(struct nf2device* nf2,
                                           const IPv4Addr& subnet,
                                           const IPv4Addr& mask,
                                           const IPv4Addr& gw,
                                           unsigned int encoded_port,
                                           unsigned int index) {
  uint32_t ip_addr_reg = ntohl(subnet.nbo());
  uint32_t mask_reg = ntohl(mask.nbo());
  uint32_t gw_reg = ntohl(gw.nbo());

  writeReg(nf2, ROUTER_OP_LUT_ROUTE_TABLE_ENTRY_IP_REG, ip_addr_reg);
  writeReg(nf2, ROUTER_OP_LUT_ROUTE_TABLE_ENTRY_MASK_REG, mask_reg);
  writeReg(nf2, ROUTER_OP_LUT_ROUTE_TABLE_ENTRY_NEXT_HOP_IP_REG, gw_reg);
  writeReg(nf2, ROUTER_OP_LUT_ROUTE_TABLE_ENTRY_OUTPUT_PORT_REG, encoded_port);
  writeReg(nf2, ROUTER_OP_LUT_ROUTE_TABLE_WR_ADDR_REG, index);
}


void HWDataPlane::initializeInterface(Interface::Ptr iface) {
  // Only initialize hardware interfaces.
  if (iface->type() != Interface::kHardware)
    return;
  // Skip already-initialized interfaces.
  if (iface->socketDescriptor() >= 0)
    return;

  // Translate "ethX" to "nf2cX".
  unsigned int index = iface->index();
  char iface_name[32] = "nf2c";
  sprintf(&(iface_name[4]), "%i", index);

  DLOG << "Initializing hardware interface " << iface_name
       << " as " << iface->name();

  int s = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
  if (s < 0) {
    perror("socket()");
    exit(1);
  }

  struct ifreq ifr;
  bzero(&ifr, sizeof(struct ifreq));
  strncpy(ifr.ifr_ifrn.ifrn_name, iface_name, IFNAMSIZ);
  if (ioctl(s, SIOCGIFINDEX, &ifr) < 0) {
    perror("ioctl SIOCGIFINDEX");
    exit(1);
  }

  struct sockaddr_ll saddr;
  bzero(&saddr, sizeof(struct sockaddr_ll));
  saddr.sll_family = AF_PACKET;
  saddr.sll_protocol = htons(ETH_P_ALL);
  saddr.sll_ifindex = ifr.ifr_ifru.ifru_ivalue;

  if (bind(s, (struct sockaddr*)(&saddr), sizeof(saddr)) < 0) {
    perror("bind error");
    exit(1);
  }

  iface->socketDescriptorIs(s);

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
  writeReg(&nf2, ROUTER_OP_LUT_MAC_0_HI_REG + (index * 0x8), mac_hi);
  writeReg(&nf2, ROUTER_OP_LUT_MAC_0_LO_REG + (index * 0x8), mac_lo);

  closeDescriptor(&nf2);
}
