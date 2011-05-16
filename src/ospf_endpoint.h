#ifndef OSPF_ENDPOINT_H_6COP8XLV
#define OSPF_ENDPOINT_H_6COP8XLV

#include "ospf_node.h"
#include "ospf_types.h"

/* OSPF endpoints are assigned a random, unoccupied router ID
   when added to an OSPFTopolgy. */

class OSPFEndpoint : public OSPFNode {
 public:
  typedef Fwk::Ptr<const OSPFEndpoint> PtrConst;
  typedef Fwk::Ptr<OSPFEndpoint> Ptr;

  static Ptr New(const RouterID& rid);

  /* Override. */
  bool isEndpoint() const { return true; }

 private:
  OSPFEndpoint(const RouterID& rid);

  /* Operations disallowed. */
  OSPFEndpoint(const OSPFEndpoint&);
  void operator=(const OSPFEndpoint&);
};

#endif
