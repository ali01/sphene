#include "gtest/gtest.h"

#include <string>
#include "ethernet_packet.h"
#include "interface.h"

using std::string;


class InterfaceTest : public ::testing::Test {
 protected:
  virtual void SetUp() {
    name_ = "eth0";
    iface_ = Interface::InterfaceNew(name_);
  }

  Interface::Ptr iface_;
  string name_;
};


TEST_F(InterfaceTest, Mac) {
  const EthernetAddr mac = "DE:AD:BE:EF:CA:FE";
  iface_->macIs(mac);
  EXPECT_EQ(mac, iface_->mac());
}
