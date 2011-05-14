#include "routing_table.h"

#include "fwk/deque.h"

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

void
RoutingTable::Entry::interfaceIs(Interface::PtrConst iface) {
  interface_ = iface;
}

bool
RoutingTable::Entry::operator==(const Entry& other) const {
  if (this->interface() != other.interface()) /* Interface ptr equivalence */
    return false;

  if (this->subnet() != other.subnet())
    return false;

  if (this->subnetMask() != other.subnetMask())
    return false;

  if (this->gateway() != other.gateway())
    return false;

  if (this->type() != other.type())
    return false;

  return true;
}

bool
RoutingTable::Entry::operator!=(const Entry& other) const {
  return !(other == *this);
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

  for (const_iterator it = entriesBegin(); it != entriesEnd(); ++it) {
    Entry::Ptr entry = it->second;

    // Routes on disabled interfaces are ignored; packets can't go out them.
    if (!entry->interface()->enabled())
      continue;

    if (entry->subnet() == (dest_ip & entry->subnetMask())) {
      if (lpm == NULL || entry->subnetMask() > lpm->subnetMask())
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

// TODO(ali): possible optimization - ignore ENTRY if it has the same
//   gateway as an existing entry but has a more specific subnet (i.e. a
//   greater subnet mask).
void
RoutingTable::entryIs(Entry::Ptr entry) {
  if (entry == NULL)
    return;

  IPv4Addr subnet = entry->subnet();
  rtable_[subnet] = entry;

  switch (entry->type()) {
    case Entry::kDynamic:
      rtable_dynamic_[subnet] = entry;
      break;

    case Entry::kStatic:
      rtable_static_[subnet] = entry;
      break;
  }

  for (unsigned int i = 0; i < notifiees_.size(); ++i)
    notifiees_[i]->onEntry(this, entry);
}

void
RoutingTable::entryDel(Entry::Ptr entry) {
  if (entry == NULL)
    return;

  IPv4Addr subnet = entry->subnet();
  rtable_.elemDel(subnet);

  switch (entry->type()) {
    case Entry::kDynamic:
      rtable_dynamic_.elemDel(subnet);
      break;
    case Entry::kStatic:
      rtable_static_.elemDel(subnet);
      break;
  }

  for (unsigned int i = 0; i < notifiees_.size(); ++i)
    notifiees_[i]->onEntryDel(this, entry);
}

void
RoutingTable::entryDel(const IPv4Addr& subnet) {
  entryDel(entry(subnet));
}

void
RoutingTable::clearDynamicEntries() {
  Fwk::Deque<IPv4Addr> del_entries;

  const_iterator it = rtable_dynamic_.begin();
  for (; it != rtable_dynamic_.end(); ++it) {
    IPv4Addr key = it->first;
    del_entries.pushBack(key);
  }

  Fwk::Deque<IPv4Addr>::const_iterator del_it;
  for (del_it = del_entries.begin(); del_it != del_entries.end(); ++del_it) {
    IPv4Addr key = *del_it;
    entryDel(key);
  }
}
