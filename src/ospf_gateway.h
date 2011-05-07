#ifndef OSPF_GATEWAY_H_CTXOEZOH
#define OSPF_GATEWAY_H_CTXOEZOH

#include "ipv4_addr.h"
#include "ospf_link.h"

/* Forward declarations. */
class OSPFNode;


class OSPFGateway : public OSPFLink {
 public:
  typedef Fwk::Ptr<const OSPFGateway> PtrConst;
  typedef Fwk::Ptr<OSPFGateway> Ptr;

  static Ptr New(Fwk::Ptr<OSPFNode> neighbor,
                 const IPv4Addr& gateway,
                 const IPv4Addr& subnet,
                 const IPv4Addr& subnet_mask);

  const IPv4Addr& gateway() const { return gateway_; }

 private:
  OSPFGateway(Fwk::Ptr<OSPFNode> neighbor,
              const IPv4Addr& gateway,
              const IPv4Addr& subnet,
              const IPv4Addr& subnet_mask);

  /* Data members. */
  IPv4Addr gateway_;

  /* Operations disallowed. */
  OSPFGateway(const OSPFGateway&);
  void operator=(const OSPFGateway&);
};

#endif
