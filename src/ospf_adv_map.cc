#include "ospf_adv_map.h"

#include "ospf_node.h"
#include "ospf_link.h"

/* OSPFAdvertisement. */

OSPFAdvertisement::OSPFAdvertisement(OSPFNode::Ptr sender, OSPFLink::Ptr link)
    : sender_(sender), link_(link) {}

OSPFAdvertisement::Ptr
OSPFAdvertisement::New(OSPFNode::Ptr sender, OSPFLink::Ptr link) {
  return new OSPFAdvertisement(sender, link);
}

OSPFNode::PtrConst
OSPFAdvertisement::sender() const {
  return sender_;
}

OSPFNode::Ptr
OSPFAdvertisement::sender() {
  return sender_;
}

OSPFLink::PtrConst
OSPFAdvertisement::link() const {
  return link_;
}

OSPFLink::Ptr
OSPFAdvertisement::link() {
  return link_;
}

/* OSPFAdvertisementMap. */
OSPFAdvertisementMap::Ptr
OSPFAdvertisementMap::New() {
  return new OSPFAdvertisementMap();
}

OSPFAdvertisement::PtrConst
OSPFAdvertisementMap::advertisement(const RouterID& sender,
                                    const RouterID& adv_nbr,
                                    const IPv4Addr& subnet,
                                    const IPv4Addr& mask) const {
  OSPFAdvertisementMap* self = const_cast<OSPFAdvertisementMap*>(this);
  return self->advertisement(sender, adv_nbr, subnet, mask);
}

OSPFAdvertisement::Ptr
OSPFAdvertisementMap::advertisement(const RouterID& sender,
                                    const RouterID& adv_nbr,
                                    const IPv4Addr& subnet,
                                    const IPv4Addr& mask) {
  AdvKey key(sender, adv_nbr, subnet, mask);
  return advs_.elem(key);
}

void
OSPFAdvertisementMap::advertisementIs(OSPFAdvertisement::Ptr adv) {
  AdvKey key(adv->sender()->routerID(),
             adv->link()->nodeRouterID(),
             adv->link()->subnet(),
             adv->link()->subnetMask());

  advs_[key] = adv;
}

void
OSPFAdvertisementMap::advertisementDel(OSPFAdvertisement::Ptr adv) {
  advertisementDel(adv->sender()->routerID(),
                   adv->link()->nodeRouterID(),
                   adv->link()->subnet(),
                   adv->link()->subnetMask());
}

void
OSPFAdvertisementMap::advertisementDel(const RouterID& sender,
                                       const RouterID& adv_nbr,
                                       const IPv4Addr& subnet,
                                       const IPv4Addr& mask) {
  AdvKey key(sender, adv_nbr, subnet, mask);
  advs_.elemDel(key);
}

/* OSPFAdvertisement::AdvKey. */

OSPFAdvertisementMap::AdvKey::AdvKey(const RouterID& sender,
                                     const RouterID& adv_nbr,
                                     const IPv4Addr& subnet,
                                     const IPv4Addr& mask)
    : sender_(sender), adv_nbr_(adv_nbr), subnet_(subnet), mask_(mask) {}

bool
OSPFAdvertisementMap::AdvKey::operator<(const AdvKey& other) const {
  if (sender_ != other.sender_)
    return sender_ < other.sender_;

  if (adv_nbr_ != other.adv_nbr_)
    return adv_nbr_ < other.adv_nbr_;

  if (subnet_ != other.subnet_)
    return subnet_ < other.subnet_;

  if (mask_ != other.mask_)
    return mask_ < other.mask_;

  /* this and other are equal. */
  return false;
}

bool
OSPFAdvertisementMap::AdvKey::operator==(const AdvKey& other) const {
  if (sender_ != other.sender_)
    return false;

  if (adv_nbr_ != other.adv_nbr_)
    return false;

  if (subnet_ != other.subnet_)
    return false;

  if (mask_ != other.mask_)
    return false;

  return true;
}
