#include "ospf_endpoint.h"

#include "ospf_constants.h"

OSPFEndpoint::Ptr
OSPFEndpoint::New(const RouterID& rid) {
  return new OSPFEndpoint(rid);
}

OSPFEndpoint::OSPFEndpoint(const RouterID& rid)
    : OSPFNode(rid) {}
