#include "routing_table.h"

#include "interface.h"

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

Interface::PtrConst
RoutingTable::Entry::interface() const {
  return interface_;
}

Interface::Ptr
RoutingTable::Entry::interface() {
  return interface_;
}

void
RoutingTable::Entry::interfaceIs(Interface::Ptr iface) {
  interface_ = iface;
}

/* RoutingTable */

RoutingTable::RoutingTable() { }

RoutingTable::Entry::Ptr
RoutingTable::entry(const IPv4Addr& subnet) {
  return rtable_.elem(subnet);
}

RoutingTable::Entry::PtrConst
RoutingTable::entry(const IPv4Addr& subnet) const {
  RoutingTable* self = const_cast<RoutingTable*>(this);
  return self->entry(subnet);
}

RoutingTable::Entry::Ptr
RoutingTable::lpm(const IPv4Addr& dest_ip) {
  Entry::Ptr lpm = NULL;

  for(const_iterator it = entriesBegin(); it != entriesEnd(); ++it) {
    Entry::Ptr entry = it->second;

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
  if (entry == NULL)
    return;

  rtable_[entry->subnet()] = entry;
}

void
RoutingTable::entryDel(Entry::Ptr entry) {
  if (entry == NULL)
    return;

  entryDel(entry->subnet());
}

void
RoutingTable::entryDel(const IPv4Addr& subnet) {
  rtable_.elemDel(subnet);
}
