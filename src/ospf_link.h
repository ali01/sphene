#ifndef OSPF_LINK_H_QJXYEZKS
#define OSPF_LINK_H_QJXYEZKS

#include <ctime>

#include "fwk/ptr_interface.h"

#include "ipv4_addr.h"
#include "ospf_types.h"

/* Forward declarations. */
class OSPFNode;

/* Class used to keep track of the subnet and subnet mask of the link on which
   a neighboring node is connected. */
class OSPFLink : public Fwk::PtrInterface<OSPFLink> {
 public:
  typedef Fwk::Ptr<const OSPFLink> PtrConst;
  typedef Fwk::Ptr<OSPFLink> Ptr;

  static Ptr New(Fwk::Ptr<OSPFNode> neighbor,
                 const IPv4Addr& subnet,
                 const IPv4Addr& subnet_mask);

  /* Accessors. */

  Fwk::Ptr<const OSPFNode> node() const;
  Fwk::Ptr<OSPFNode> node();
  const RouterID& nodeRouterID() const;
  bool nodeIsEndpoint() const;

  const IPv4Addr& subnet() const { return subnet_; }
  const IPv4Addr& subnetMask() const { return subnet_mask_; }

  time_t timeSinceLSU() const { return time(NULL) - last_lsu_; }
  void timeSinceLSUIs(time_t _t) { last_lsu_ = time(NULL) - _t; }

  /* Comparison operator. */
  bool operator==(const OSPFLink&) const;
  bool operator!=(const OSPFLink&) const;

 protected:
  OSPFLink(Fwk::Ptr<OSPFNode> neighbor,
           const IPv4Addr& subnet,
           const IPv4Addr& subnet_mask);
 private:
  /* Data members. */
  Fwk::Ptr<OSPFNode> node_;
  IPv4Addr subnet_;
  IPv4Addr subnet_mask_;

  /* Time this link was last confirmed by a link-state update. */
  time_t last_lsu_;

  /* Operations disallowed. */
  OSPFLink(const OSPFLink&);
  void operator=(const OSPFLink&);
};

#endif
