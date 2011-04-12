#include "routing_table.h"

#include <pthread.h>


/* RoutingTable::Entry */

RoutingTable::Entry::Entry()
    : subnet_((uint32_t)0), subnet_mask_((uint32_t)0), gateway_((uint32_t)0),
      interface_(NULL) {}

void
RoutingTable::Entry::subnetIs(const IPv4Addr& dest_ip,
                              const IPv4Addr& subnet_mask) {
  subnet_ = dest_ip & subnet_mask;
  subnet_mask_ = subnet_mask;
}


/* RoutingTable */

RoutingTable::RoutingTable() {
  // TODO(ms): Check return value.
  pthread_mutex_init(&lock_, NULL);
}


RoutingTable::Entry::Ptr
RoutingTable::lpm(const IPv4Addr& dest_ip) const {
  Entry::Ptr lpm = NULL;
  Entry::Ptr curr = rtable_.front();
  while (curr)  {
    if (curr->subnet() == (dest_ip & curr->subnetMask())) {
      if (lpm == NULL || curr->subnetMask() > lpm->subnetMask())
        lpm = curr;
    }
    curr = curr->next();
  }

  return lpm;
}


void
RoutingTable::lockedIs(const bool locked) {
  if (locked)
    pthread_mutex_lock(&lock_);
  else
    pthread_mutex_unlock(&lock_);
}
