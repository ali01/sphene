#include "routing_table.h"

#include "fwk/deque.h"

#include "interface.h"

// RoutingTable

RoutingTable::RoutingTable(InterfaceMap::Ptr iface_map)
    : iface_map_reactor_(InterfaceMapReactor::New(this)) {

  if (iface_map) {
    iface_map_reactor_->notifierIs(iface_map);

    // Process existing interfaces in IFACE_MAP.
    InterfaceMap::const_iterator it;
    for (it = iface_map->begin(); it != iface_map->end(); ++it) {
      Interface::Ptr iface = it->second;
      iface_map_reactor_->onInterface(iface_map, iface);
    }
  }
}

RoutingTable::Entry::Ptr
RoutingTable::entry(const IPv4Addr& subnet, const IPv4Addr& mask) {
  return rtable_.elem(std::make_pair(subnet, mask));
}

RoutingTable::Entry::PtrConst
RoutingTable::entry(const IPv4Addr& subnet, const IPv4Addr& mask) const {
  RoutingTable* self = const_cast<RoutingTable*>(this);
  return self->entry(subnet, mask);
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

void
RoutingTable::entryIs(Entry::Ptr entry) {
  if (entry == NULL)
    return;

  IPv4Addr subnet = entry->subnet();
  IPv4Addr mask = entry->subnetMask();

  Entry::Ptr prev_entry = this->entry(subnet, mask);

  IPv4Subnet key = std::make_pair(subnet, mask);
  rtable_[key] = entry;

  switch (entry->type()) {
    case Entry::kDynamic:
      rtable_dynamic_[key] = entry;
      break;

    case Entry::kStatic:
      rtable_static_[key] = entry;
      break;
  }

  if (prev_entry == NULL || *entry != *prev_entry) {
    for (unsigned int i = 0; i < notifiees_.size(); ++i)
      notifiees_[i]->onEntry(this, entry);
  }
}

void
RoutingTable::entryDel(Entry::Ptr entry) {
  if (entry == NULL)
    return;

  IPv4Addr subnet = entry->subnet();
  IPv4Addr mask = entry->subnetMask();

  entry = this->entry(subnet, mask);

  // Only attempt perform deletion (and trigger notification),
  // if ENTRY is actually in the routing table.
  if (entry) {
    IPv4Subnet key = std::make_pair(subnet, mask);
    rtable_.elemDel(key);

    switch (entry->type()) {
      case Entry::kDynamic:
        rtable_dynamic_.elemDel(key);
        break;

      case Entry::kStatic:
        rtable_static_.elemDel(key);
        break;
    }

    for (unsigned int i = 0; i < notifiees_.size(); ++i)
      notifiees_[i]->onEntryDel(this, entry);
  }
}

void
RoutingTable::entryDel(const IPv4Addr& subnet, const IPv4Addr& mask) {
  entryDel(entry(subnet, mask));
}

void
RoutingTable::clearDynamicEntries() {
  Fwk::Deque<IPv4Subnet> del_entries;

  const_iterator it = rtable_dynamic_.begin();
  for (; it != rtable_dynamic_.end(); ++it) {
    IPv4Subnet key = it->first;
    del_entries.pushBack(key);
  }

  Fwk::Deque<IPv4Subnet>::const_iterator del_it;
  for (del_it = del_entries.begin(); del_it != del_entries.end(); ++del_it) {
    IPv4Subnet key = *del_it;
    entryDel(key.first, key.second);
  }
}


// RoutingTable::Entry

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


// RoutingTable::InterfaceMapReactor

void
RoutingTable::InterfaceMapReactor::onInterface(InterfaceMap::Ptr map,
                                               Interface::Ptr iface) {
  Entry::Ptr entry = Entry::New(Entry::kStatic);
  entry->subnetIs(iface->subnet(), iface->subnetMask());
  entry->gatewayIs(IPv4Addr::kZero);
  entry->interfaceIs(iface);
  rtable_->entryIs(entry);
}

void
RoutingTable::InterfaceMapReactor::onInterfaceDel(InterfaceMap::Ptr map,
                                                  Interface::Ptr iface) {
  rtable_->entryDel(iface->subnet(), iface->subnetMask());
}
