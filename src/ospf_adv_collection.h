#ifndef OSPF_ADV_COLLECTION_H_V5F9TPQ9
#define OSPF_ADV_COLLECTION_H_V5F9TPQ9

#include "fwk/ptr_interface.h"

#include "ospf_router_id.h"
#include "ipv4_addr.h"


class OSPFAdvertisementCollection
    : public Fwk::PtrInterface<OSPFAdvertisementCollection> {
 public:
  typedef Fwk::Ptr<const OSPFAdvertisementCollection> PtrConst;
  typedef Fwk::Ptr<OSPFAdvertisementCollection> Ptr;

  virtual size_t advertisements() const = 0;

 protected:
  struct AdvKey {
    AdvKey(const RouterID& sender,
           const RouterID& adv_nbr,
           const IPv4Addr& subnet,
           const IPv4Addr& mask);

    RouterID sender_;
    RouterID adv_nbr_;
    IPv4Addr subnet_;
    IPv4Addr mask_;

    bool operator<(const AdvKey&) const;
    bool operator==(const AdvKey&) const;
  };

  OSPFAdvertisementCollection() {}
  virtual ~OSPFAdvertisementCollection() {}

 private:

  /* Operations disallowed. */
  OSPFAdvertisementCollection(const OSPFAdvertisementCollection&);
  void operator=(const OSPFAdvertisementCollection&);
};

#endif
