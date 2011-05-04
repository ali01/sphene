#include "routing_table.h"

/* RoutingTable::Entry */

RoutingTable::Entry::Entry(Type type)
    : subnet_((uint32_t)0), subnet_mask_((uint32_t)0), gateway_((uint32_t)0),
      interface_(NULL), type_(type) {}

void
RoutingTable::Entry::subnetIs(const IPv4Addr& dest_ip,
                              const IPv4Addr& subnet_mask) {
  subnet_ = dest_ip & subnet_mask;
  subnet_mask_ = subnet_mask;
}


/* RoutingTable */

RoutingTable::RoutingTable() { }


RoutingTable::Entry::Ptr
RoutingTable::lpm(const IPv4Addr& dest_ip) {
  Entry::Ptr lpm = NULL;
  Entry::Ptr entry;
  for (entry = rtable_.front(); entry; entry = entry->next()) {
    // Routes on disabled interfaces are ignored; packets can't go out them.
    if (!entry->interface()->enabled())
      continue;

    if (entry->subnet() == (dest_ip & entry->subnetMask())) {
      if (lpm == NULL || entry->subnetMask() > entry->subnetMask())
        lpm = entry;
    }
  }

  return lpm;
}

RoutingTable::Entry::PtrConst
RoutingTable::lpm(const IPv4Addr& dest_ip) const {
  RoutingTable* self = const_cast<RoutingTable*>(this);
  return self->lpm(dest_ip);
}

void
RoutingTable::entryIs(Entry::Ptr entry) {
  Entry::Ptr existing = lpm(entry->subnet());
  if (existing && entry->subnetMask() == existing->subnetMask()) {
    // We have an existing routing entry for this subnet; remove it.
    entryDel(existing);
  }

  // Add the new entry.
  rtable_.pushFront(entry);
}
