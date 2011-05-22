#include "ospf_adv_set.h"

OSPFAdvertisementSet::Ptr
OSPFAdvertisementSet::New() {
  return new OSPFAdvertisementSet();
}

bool
OSPFAdvertisementSet::contains(const RouterID& sender,
                               const RouterID& adv_nbr,
                               const IPv4Addr& subnet,
                               const IPv4Addr& mask) const {
  AdvKey key(sender, adv_nbr, subnet, mask);
  return advs_.elem(key) != advs_.end();
}

void
OSPFAdvertisementSet::advertisementIs(const RouterID& sender,
                                      const RouterID& adv_nbr,
                                      const IPv4Addr& subnet,
                                      const IPv4Addr& mask) {
  AdvKey key(sender, adv_nbr, subnet, mask);
  advs_.elemIs(key);
}

void
OSPFAdvertisementSet::advertisementDel(const RouterID& sender,
                                       const RouterID& adv_nbr,
                                       const IPv4Addr& subnet,
                                       const IPv4Addr& mask) {
  AdvKey key(sender, adv_nbr, subnet, mask);
  advs_.elemDel(key);
}
