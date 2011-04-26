#ifndef OSPF_NEIGHBOR_H_MVB24XTS
#define OSPF_NEIGHBOR_H_MVB24XTS

#include <ctime>

#include "fwk/ptr_interface.h"

#include "ipv4_addr.h"

class OSPFNeighbor : public Fwk::PtrInterface<OSPFNeighbor> {
 public:
  typedef Fwk::Ptr<const OSPFNeighbor> PtrConst;
  typedef Fwk::Ptr<OSPFNeighbor> Ptr;

  /* Expects and returns all values in host byte-order. */
  static Ptr New(uint32_t id, IPv4Addr addr) {
    return new OSPFNeighbor(id, addr);
  }

  uint32_t routerId() const { return id_; }
  IPv4Addr ipAddr() const { return ip_addr_; }

  time_t age() const { return time(NULL) - t_; }
  void ageIs(time_t age) { t_ = time(NULL) - age; }

 private:
  OSPFNeighbor(uint32_t id, IPv4Addr addr);

  /* Data members. */
  uint32_t id_;
  IPv4Addr ip_addr_;
  time_t t_;

  /* Operations disallowed. */
  OSPFNeighbor(const OSPFNeighbor&);
  void operator=(const OSPFNeighbor&);
};

#endif
