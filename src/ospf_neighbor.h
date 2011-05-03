#ifndef OSPF_NEIGHBOR_H_QJXYEZKS
#define OSPF_NEIGHBOR_H_QJXYEZKS

#include "fwk/ptr_interface.h"

#include "ipv4_addr.h"

/* Forward declarations. */
class OSPFNode;

/* Class used to keep track of the subnet and subnet mask of the link on which
   a neighboring node is connected. */
class OSPFNeighbor : public Fwk::PtrInterface<OSPFNeighbor> {
 public:
  typedef Fwk::Ptr<const OSPFNeighbor> PtrConst;
  typedef Fwk::Ptr<OSPFNeighbor> Ptr;

  static Ptr New(Fwk::Ptr<OSPFNode> neighbor,
                 const IPv4Addr& subnet,
                 const IPv4Addr& subnet_mask);

  /* Accessors. */

  Fwk::Ptr<const OSPFNode> node() const;
  Fwk::Ptr<OSPFNode> node();

  const IPv4Addr& subnet() const { return subnet_; }
  const IPv4Addr& subnetMask() const { return subnet_mask_; }

 private:
  OSPFNeighbor(Fwk::Ptr<OSPFNode> neighbor,
               const IPv4Addr& subnet,
               const IPv4Addr& subnet_mask);

  /* Data members. */
  Fwk::Ptr<OSPFNode> node_;
  IPv4Addr subnet_;
  IPv4Addr subnet_mask_;

  /* Operations disallowed. */
  OSPFNeighbor(const OSPFNeighbor&);
  void operator=(const OSPFNeighbor&);
};

#endif
