#ifndef OSPF_GATEWAY_H_CTXOEZOH
#define OSPF_GATEWAY_H_CTXOEZOH

#include <ctime>

#include "ipv4_addr.h"
#include "ospf_link.h"

/* Forward declarations. */
class OSPFNode;
class OSPFInterface;

class OSPFGateway : public OSPFLink {
 public:
  typedef Fwk::Ptr<const OSPFGateway> PtrConst;
  typedef Fwk::Ptr<OSPFGateway> Ptr;

  static Ptr New(Fwk::Ptr<OSPFNode> neighbor,
                 const IPv4Addr& gateway,
                 const IPv4Addr& subnet,
                 const IPv4Addr& subnet_mask);

  static Ptr NewPassive(const IPv4Addr& gateway,
                        const IPv4Addr& subnet,
                        const IPv4Addr& mask);

  const IPv4Addr& gateway() const { return gateway_; }
  void gatewayIs(const IPv4Addr& gw) { gateway_ = gw; }

  time_t timeSinceHello() const { return time(NULL) - last_hello_; }
  void timeSinceHelloIs(time_t _t) { last_hello_ = time(NULL) - _t; }

  const OSPFInterface* interface() const;
  OSPFInterface* interface();
  void interfaceIs(OSPFInterface* iface);

  /* Comparison operator. */
  bool operator==(const OSPFGateway&) const;
  bool operator!=(const OSPFGateway&) const;

 private:
  OSPFGateway(Fwk::Ptr<OSPFNode> neighbor,
              const IPv4Addr& gateway,
              const IPv4Addr& subnet,
              const IPv4Addr& subnet_mask);

  OSPFGateway(const IPv4Addr& gateway,
              const IPv4Addr& subnet,
              const IPv4Addr& mask);

  /* Data members. */
  IPv4Addr gateway_;
  time_t last_hello_;
  OSPFInterface* iface_;

  /* Operations disallowed. */
  OSPFGateway(const OSPFGateway&);
  void operator=(const OSPFGateway&);
};

#endif
