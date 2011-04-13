#include "arp_queue.h"

void
ARPQueue::Entry::packetIs(IPPacket::Ptr packet) {
  PacketWrapper::Ptr wrapper = PacketWrapper::New(packet);
  packet_queue_.pushBack(wrapper);
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
