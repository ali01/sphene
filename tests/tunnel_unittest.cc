#include "gtest/gtest.h"

#include <string>
#include "interface.h"
#include "ipv4_addr.h"
#include "tunnel.h"

using std::string;


class TunnelTest : public ::testing::Test {
 protected:
  virtual void SetUp() {
    name_ = "eth0";
    iface_ = Interface::InterfaceNew(name_);
    tunnel_ = Tunnel::New(iface_);
  }

  Interface::Ptr iface_;
  Tunnel::Ptr tunnel_;
  string name_;
};


TEST_F(TunnelTest, Construct) {
  EXPECT_EQ(name_, tunnel_->name());
  EXPECT_EQ(iface_.ptr(), tunnel_->interface().ptr());
}


TEST_F(TunnelTest, remote) {
  // Tunnel is created with no remote.
  EXPECT_EQ(string("0.0.0.0"), (string)tunnel_->remote());

  // Change the remote.
  const IPv4Addr remote = "4.2.2.1";
  tunnel_->remoteIs(remote);
  EXPECT_EQ(remote, tunnel_->remote());
}


TEST_F(TunnelTest, mode) {
  // We only support GRE for now.
  EXPECT_EQ(Tunnel::kGRE, tunnel_->mode());
}
