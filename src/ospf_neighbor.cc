#include "ospf_neighbor.h"

OSPFNeighbor::OSPFNeighbor(uint32_t id, IPv4Addr iface_addr)
    : id_(id), iface_addr_(iface_addr) {}
