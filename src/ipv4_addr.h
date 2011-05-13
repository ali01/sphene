#ifndef IPV4_ADDR_H_PDYPHF3C
#define IPV4_ADDR_H_PDYPHF3C

#include <arpa/inet.h>
#include <netinet/in.h>
#include <inttypes.h>
#include <string>

#include "fwk/atomic.h"
#include "fwk/ordinal.h"


class IPv4Addr {
 public:
  static const IPv4Addr kMax;
  static const IPv4Addr kZero;

  IPv4Addr();

  /* Expects an IP address in host byte order */
  IPv4Addr(uint32_t addr);

  /* Construct from the dotted string representation. */
  IPv4Addr(const std::string& addr);
  IPv4Addr(const char* addr);

  /* Masking operators. */
  IPv4Addr operator&(uint32_t other) const;
  IPv4Addr operator&(const IPv4Addr& other) const;

  IPv4Addr& operator&=(uint32_t other);
  IPv4Addr& operator&=(const IPv4Addr& other);

  /* Comparison operators. */
  bool operator==(const IPv4Addr& other) const;
  bool operator!=(const IPv4Addr& other) const;
  bool operator<=(const IPv4Addr& other) const;
  bool operator>=(const IPv4Addr& other) const;
  bool operator< (const IPv4Addr& other) const;
  bool operator> (const IPv4Addr& other) const;

  /* Returns IP address in network byte order */
  uint32_t nbo() const;

  /* Returns IP address in host byte order. */
  uint32_t value() const { return addr_.value(); }

  /* Returns string representation of IP address. */
  operator std::string() const;

  /* Address length in bytes. */
  static const int kAddrLen = 4;

 private:
  Fwk::AtomicUInt32 addr_;
};

#endif
