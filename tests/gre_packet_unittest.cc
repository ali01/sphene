#include "gtest/gtest.h"

#include <arpa/inet.h>
#include <cstring>
#include <inttypes.h>
#include <string>

#include "gre_packet.h"
#include "fwk/buffer.h"
#include "fwk/exception.h"

using std::string;


namespace {

struct GREHeader {
  uint16_t c_resv0_ver;  // C bit, Reserved0, Ver
  uint16_t ptype;        // protocol type
  uint16_t cksum;        // checksum of header and payload (optional)
  uint16_t resv1;        // Reserved1 (optional)
} __attribute__((packed));

}  // namespace


class GREPacketTest : public ::testing::Test {
 protected:
  virtual void SetUp() {
    uint8_t gre_packet[] = { 0x00,          // Checksum not present
                             0x00,          // Version = 0
                             0x08, 0x00 };  // Ptype = IP

    // Put a packet in a buffer.
    buf_ = Fwk::Buffer::BufferNew(gre_packet, sizeof(gre_packet));

    // Update header pointer.
    header_ = (struct GREHeader*)buf_->data();

    // Construct GREPacket from buffer.
    pkt_ = GREPacket::GREPacketNew(buf_, 0);
  }

  Fwk::Buffer::Ptr buf_;
  struct GREHeader* header_;
  GREPacket::Ptr pkt_;
};
