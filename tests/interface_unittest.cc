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
  const EthernetAddr& mac = "DE:AD:BE:EF:CA:FE";
  iface_->macIs(mac);
  EXPECT_EQ(mac, iface_->mac());
}


TEST_F(InterfaceTest, IP) {
  const IPv4Addr& ip = "255.1.33.7";
  iface_->ipIs(ip);
  EXPECT_EQ(ip, iface_->ip());
}


TEST_F(InterfaceTest, SubnetMask) {
  const IPv4Addr& mask = "255.1.33.7";
  iface_->subnetMaskIs(mask);
  EXPECT_EQ(mask, iface_->subnetMask());
}


TEST_F(InterfaceTest, BroadcastAddr) {
  iface_->ipIs("10.1.0.42");
  iface_->subnetMaskIs("255.255.255.0");
  EXPECT_EQ(IPv4Addr("10.1.0.255"), iface_->broadcastAddr());

  iface_->ipIs("0.0.0.0");
  iface_->subnetMaskIs("0.0.0.0");
  EXPECT_EQ(IPv4Addr("255.255.255.255"), iface_->broadcastAddr());

  iface_->ipIs("172.16.1.145");
  iface_->subnetMaskIs("255.240.0.0");
  EXPECT_EQ(IPv4Addr("172.31.255.255"), iface_->broadcastAddr());
}


TEST_F(InterfaceTest, Speed) {
  const uint32_t speed = 31337;
  iface_->speedIs(speed);
  EXPECT_EQ(speed, iface_->speed());
}


TEST_F(InterfaceTest, Enabled) {
  bool enabled = true;
  iface_->enabledIs(enabled);
  EXPECT_EQ(enabled, iface_->enabled());

  enabled = false;
  iface_->enabledIs(enabled);
  EXPECT_EQ(enabled, iface_->enabled());
}
