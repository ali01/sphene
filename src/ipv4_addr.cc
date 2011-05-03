#include "ipv4_addr.h"

/* IPv4Addr */

const IPv4Addr IPv4Addr::kMax = (uint32_t)0xffffffff;
const IPv4Addr IPv4Addr::kZero = (uint32_t)0x0;
const int IPv4Addr::kAddrLen;


IPv4Addr::IPv4Addr() : Fwk::Ordinal<IPv4Addr,uint32_t>(0) {}

IPv4Addr::IPv4Addr(uint32_t addr) : Fwk::Ordinal<IPv4Addr,uint32_t>(addr) {}

IPv4Addr::IPv4Addr(const std::string& addr) {
  uint32_t nbo;
  if (inet_pton(AF_INET, addr.c_str(), &nbo))
    valueIs(ntohl(nbo));
  else
    valueIs(0);
}

IPv4Addr::IPv4Addr(const char* const addr) {
  uint32_t nbo;
  if (inet_pton(AF_INET, addr, &nbo))
    valueIs(ntohl(nbo));
  else
    valueIs(0);
}

IPv4Addr
IPv4Addr::operator&(uint32_t other) const {
  return value() & other;
}

IPv4Addr
IPv4Addr::operator&(const IPv4Addr& other) const {
  return value() & other.value();
}

IPv4Addr&
IPv4Addr::operator&=(uint32_t other) {
  valueIs(value() & other);
  return *this;
}

IPv4Addr&
IPv4Addr::operator&=(const IPv4Addr& other) {
  valueIs(value() & other.value());
  return *this;
}

/* Returns IP address in network byte order */
uint32_t
IPv4Addr::nbo() const {
  return htonl(value());
}

IPv4Addr::operator std::string() const {
  uint32_t nbo = this->nbo();
  char buf[INET_ADDRSTRLEN + 1];
  inet_ntop(AF_INET, (struct in_addr*)&nbo, buf, sizeof(buf));
  buf[INET_ADDRSTRLEN] = 0;
  return buf;
}
