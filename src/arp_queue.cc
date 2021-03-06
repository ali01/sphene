#include "arp_queue.h"

/* ARPQueue::Entry */

void
ARPQueue::Entry::packetIs(EthernetPacket::Ptr packet) {
  PacketWrapper::Ptr wrapper = PacketWrapper::New(packet);
  packet_queue_.pushBack(wrapper);
}


/* ARPQueue */

ARPQueue::Entry::Ptr
ARPQueue::entry(const IPv4Addr& ip) const {
  Entry::Ptr entry = NULL;
  const_iterator it = addr_map_.find(ip);
  if (it != this->end())
    entry = it->second;

  return entry;
}

void
ARPQueue::entryIs(Entry::Ptr entry) {
  if (entry) {
    IPv4Addr key = entry->ipAddr();
    addr_map_[key] = entry;
  }
}

void
ARPQueue::entryDel(Entry::Ptr entry) {
  if (entry == NULL)
    return;

  IPv4Addr key = entry->ipAddr();
  this->entryDel(key);
}

void
ARPQueue::entryDel(const IPv4Addr& ip) {
  addr_map_.erase(ip);
}
