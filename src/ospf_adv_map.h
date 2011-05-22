#ifndef OSPF_ADV_MAP_H_17AYHYOJ
#define OSPF_ADV_MAP_H_17AYHYOJ

#include "fwk/map.h"

#include "ospf_adv_collection.h"

/* Forward declarations. */
class OSPFLink;
class OSPFNode;

/* OSPFAdvertisement. */

class OSPFAdvertisement : public Fwk::PtrInterface<OSPFAdvertisement> {
 public:
  typedef Fwk::Ptr<const OSPFAdvertisement> PtrConst;
  typedef Fwk::Ptr<OSPFAdvertisement> Ptr;

  static Ptr New(Fwk::Ptr<OSPFNode> sender, Fwk::Ptr<OSPFLink> link);

  Fwk::Ptr<const OSPFNode> sender() const;
  Fwk::Ptr<OSPFNode> sender();

  Fwk::Ptr<const OSPFLink> link() const;
  Fwk::Ptr<OSPFLink> link();

 private:
  OSPFAdvertisement(Fwk::Ptr<OSPFNode> sender, Fwk::Ptr<OSPFLink> link);

  /* Data members. */
  Fwk::Ptr<OSPFNode> sender_;
  Fwk::Ptr<OSPFLink> link_;

  /* Operations disallowed. */
  OSPFAdvertisement(const OSPFAdvertisement&);
  void operator=(const OSPFAdvertisement&);
};

/* OSPFAdvertisementMap. */

class OSPFAdvertisementMap : public OSPFAdvertisementCollection {
 public:
  typedef Fwk::Ptr<const OSPFAdvertisementMap> PtrConst;
  typedef Fwk::Ptr<OSPFAdvertisementMap> Ptr;

  static Ptr New();
  OSPFAdvertisementMap() {}

  OSPFAdvertisement::PtrConst advertisement(const RouterID& sender,
                                            const RouterID& adv_nbr,
                                            const IPv4Addr& subnet,
                                            const IPv4Addr& mask) const;

  OSPFAdvertisement::Ptr advertisement(const RouterID& sender,
                                       const RouterID& adv_nbr,
                                       const IPv4Addr& subnet,
                                       const IPv4Addr& mask);

  void advertisementIs(OSPFAdvertisement::Ptr adv);
  void advertisementDel(OSPFAdvertisement::Ptr adv);
  void advertisementDel(const RouterID& sender,
                        const RouterID& adv_nbr,
                        const IPv4Addr& subnet,
                        const IPv4Addr& mask);

  size_t advertisements() const { return advs_.size(); }

 private:
  /* Data members. */
  Fwk::Map<AdvKey,OSPFAdvertisement> advs_;

  /* Operations disallowed. */
  OSPFAdvertisementMap(const OSPFAdvertisementMap&);
  void operator=(const OSPFAdvertisementMap&);
};

#endif
