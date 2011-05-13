#include "fwk/atomic.h"
#include "ipv4_addr.h"

/* IPv4Addr */

const IPv4Addr IPv4Addr::kMax = (uint32_t)0xffffffff;
const IPv4Addr IPv4Addr::kZero = (uint32_t)0x0;
const int IPv4Addr::kAddrLen;


IPv4Addr::IPv4Addr()
    : addr_(0) { }

IPv4Addr::IPv4Addr(uint32_t addr)
    : addr_(addr) { }

IPv4Addr::IPv4Addr(const std::string& addr) {
  uint32_t nbo;
  if (inet_pton(AF_INET, addr.c_str(), &nbo))
    addr_ = ntohl(nbo);
  else
    addr_ = 0;
}

IPv4Addr::IPv4Addr(const char* const addr) {
  uint32_t nbo;
  if (inet_pton(AF_INET, addr, &nbo))
    addr_ = ntohl(nbo);
  else
    addr_ = 0;
}

IPv4Addr
IPv4Addr::operator&(uint32_t other) const {
  return addr_ & other;
}

IPv4Addr
IPv4Addr::operator&(const IPv4Addr& other) const {
  return addr_ & other.addr_;
}

IPv4Addr&
IPv4Addr::operator&=(uint32_t other) {
  addr_ &= other;
  return *this;
}

IPv4Addr&
IPv4Addr::operator&=(const IPv4Addr& other) {
  addr_ &= other.addr_;
  return *this;
}

bool
IPv4Addr::operator==(const IPv4Addr& other) const {
  return (addr_ == other.addr_);
}

bool
IPv4Addr::operator!=(const IPv4Addr& other) const {
  return (addr_ != other.addr_);
}

bool
IPv4Addr::operator<=(const IPv4Addr& other) const {
  return (addr_ <= other.addr_);
}

bool
IPv4Addr::operator>=(const IPv4Addr& other) const {
  return (addr_ >= other.addr_);
}

bool
IPv4Addr::operator<(const IPv4Addr& other) const {
  return (addr_ < other.addr_);
}

bool
IPv4Addr::operator>(const IPv4Addr& other) const {
  return (addr_ > other.addr_);
}


/* Returns IP address in network byte order */
uint32_t
IPv4Addr::nbo() const {
  return htonl(addr_.value());
}

IPv4Addr::operator std::string() const {
  uint32_t nbo = this->nbo();
  char buf[INET_ADDRSTRLEN + 1];
  inet_ntop(AF_INET, (struct in_addr*)&nbo, buf, sizeof(buf));
  buf[INET_ADDRSTRLEN] = 0;
  return buf;
}
