#ifndef OSPF_ADV_SET_H_ISFXAIWQ
#define OSPF_ADV_SET_H_ISFXAIWQ

#include "fwk/set.h"

#include "ipv4_addr.h"
#include "ospf_adv_collection.h"
#include "ospf_router_id.h"


class OSPFAdvertisementSet : public OSPFAdvertisementCollection {
 public:
  typedef Fwk::Ptr<const OSPFAdvertisementSet> PtrConst;
  typedef Fwk::Ptr<OSPFAdvertisementSet> Ptr;

  static Ptr New();
  OSPFAdvertisementSet() {}

  bool contains(const RouterID& sender,
                const RouterID& adv_nbr,
                const IPv4Addr& subnet,
                const IPv4Addr& mask) const;

  void advertisementIs(const RouterID& sender,
                       const RouterID& adv_nbr,
                       const IPv4Addr& subnet,
                       const IPv4Addr& mask);

  void advertisementDel(const RouterID& sender,
                        const RouterID& adv_nbr,
                        const IPv4Addr& subnet,
                        const IPv4Addr& mask);

  size_t advertisements() const { return advs_.size(); }

 private:

  /* Data members. */
  Fwk::Set<AdvKey> advs_;

  /* Operations disallowed. */
  OSPFAdvertisementSet(const OSPFAdvertisementSet&);
  void operator=(const OSPFAdvertisementSet&);
};

#endif
