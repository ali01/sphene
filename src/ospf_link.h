#ifndef OSPF_LINK_H_QJXYEZKS
#define OSPF_LINK_H_QJXYEZKS

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

  const IPv4Addr& subnet() const { return subnet_; }
  const IPv4Addr& subnetMask() const { return subnet_mask_; }

 protected:
  OSPFLink(Fwk::Ptr<OSPFNode> neighbor,
               const IPv4Addr& subnet,
               const IPv4Addr& subnet_mask);
 private:
  /* Data members. */
  Fwk::Ptr<OSPFNode> node_;
  IPv4Addr subnet_;
  IPv4Addr subnet_mask_;

  /* Operations disallowed. */
  OSPFLink(const OSPFLink&);
  void operator=(const OSPFLink&);
};

#endif
