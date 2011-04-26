#ifndef OSPF_NEIGHBOR_H_MVB24XTS
#define OSPF_NEIGHBOR_H_MVB24XTS

#include "fwk/ptr_interface.h"

#include "ipv4_addr.h"

class OSPFNeighbor : public Fwk::PtrInterface<OSPFNeighbor> {
 public:
  typedef Fwk::Ptr<const OSPFNeighbor> PtrConst;

  /* Expects and returns all values in host byte-order. */
  static PtrConst New(uint32_t id, IPv4Addr iface_addr) {
    return new OSPFNeighbor(id, iface_addr);
  }

  uint32_t routerId() const { return id_; }
  IPv4Addr interfaceAddr() const { return iface_addr_; }

 private:
  OSPFNeighbor(uint32_t id, IPv4Addr iface_addr);

  /* Data members. */
  uint32_t id_;
  IPv4Addr iface_addr_;

  /* Operations disallowed. */
  OSPFNeighbor(const OSPFNeighbor&);
  void operator=(const OSPFNeighbor&);
};

#endif
