#include "ospf_neighbor.h"

OSPFNeighbor::OSPFNeighbor(uint32_t id, IPv4Addr addr)
    : id_(id), ip_addr_(addr) {}
