#include "routing_table.h"

/* RoutingTable::Entry */

RoutingTable::Entry::Entry()
    : subnet_((uint32_t)0), subnet_mask_((uint32_t)0), gateway_((uint32_t)0),
      interface_(NULL), next_(NULL), prev_(NULL) {}

void
RoutingTable::Entry::subnetIs(const IPv4Addr& dest_ip,
                              const IPv4Addr& subnet_mask) {
  subnet_ = dest_ip & subnet_mask;
  subnet_mask_ = subnet_mask;
}

/* RoutingTable */

RoutingTable::Entry::Ptr
RoutingTable::longestPrefixMatch(const IPv4Addr& dest_ip) const {
  Entry::Ptr lpm = NULL;
  Entry::Ptr curr = rtable_;
  while (curr)  {
    if (curr->subnet() == (dest_ip & curr->subnetMask())) {
      if (lpm == NULL || curr->subnetMask() > lpm->subnetMask())
        lpm = curr;
    }
    curr = curr->next_;
  }

  return lpm;
}

void
RoutingTable::entryIs(Entry::Ptr entry) {
  if (entry == NULL)
    return;

  /* If entry is already in the routing table. */
  if (rtable_ && rtable_set_.find(entry.ptr()) != rtable_set_.end())
    return;

  rtable_set_.insert(entry.ptr());

  if (rtable_)
    rtable_->prev_ = entry.ptr();

  entry->prev_ = NULL;
  entry->next_ = rtable_;
  rtable_ = entry;
}

void
RoutingTable::entryDel(Entry::Ptr entry) {
  if (entry == NULL || rtable_ == NULL)
    return;

  /* If entry is the root node. */
  if (rtable_ == entry)
    rtable_ = entry->next_;
  else
    entry->prev_->next_ = entry->next_;

  /* If entry is not the tail. */
  if (entry->next_)
    entry->next_->prev_ = entry->prev_;

  rtable_set_.erase(entry.ptr());
}
