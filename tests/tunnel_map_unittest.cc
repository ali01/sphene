#include "gtest/gtest.h"

#include <string>
#include "interface.h"
#include "ipv4_addr.h"
#include "tunnel.h"
#include "tunnel_map.h"

using std::string;


class TunnelMapTest : public ::testing::Test {
 protected:
  virtual void SetUp() {
    map_ = TunnelMap::New();
    eth0_ = Interface::InterfaceNew("eth0");
    eth0_->ipIs("8.8.4.4");
    tun0_ = Tunnel::New(eth0_);
    tun0_->remoteIs("192.168.1.1");

    eth1_ = Interface::InterfaceNew("eth1");
    eth1_->ipIs("4.2.2.1");
    tun1_ = Tunnel::New(eth1_);
    tun1_->remoteIs("10.0.0.1");
  }

  Interface::Ptr eth0_;
  Tunnel::Ptr tun0_;
  Interface::Ptr eth1_;
  Tunnel::Ptr tun1_;
  TunnelMap::Ptr map_;
};


TEST_F(TunnelMapTest, Insert) {
  // Map is initially empty.
  EXPECT_EQ((size_t)0, map_->tunnels());

  // Insert tunnels into the map.
  map_->tunnelIs(tun0_);
  map_->tunnelIs(tun1_);

  // Ensure the map size has increased.
  EXPECT_EQ((size_t)2, map_->tunnels());
}


TEST_F(TunnelMapTest, Retrieve) {
  // Insert tunnels.
  map_->tunnelIs(tun0_);
  map_->tunnelIs(tun1_);

  // Ensure we can retrieve them by name.
  EXPECT_EQ(tun0_, map_->tunnel(tun0_->name()));
  EXPECT_EQ(tun1_, map_->tunnel(tun1_->name()));

  // Ensure we can retrieve them by remote IP address.
  EXPECT_EQ(tun0_, map_->tunnelRemoteAddr(tun0_->remote()));
  EXPECT_EQ(tun1_, map_->tunnelRemoteAddr(tun1_->remote()));

  // NULL pointers are returned for non-existent tunnels.
  Tunnel::Ptr null(NULL);
  EXPECT_EQ(null, map_->tunnel("bogusiface"));
}


TEST_F(TunnelMapTest, Delete) {
  Tunnel::Ptr null(NULL);

  // Insert tunnels.
  map_->tunnelIs(tun0_);
  map_->tunnelIs(tun1_);
  ASSERT_EQ((size_t)2, map_->tunnels());

  // Delete one tunnel.
  map_->tunnelDel(tun0_);
  EXPECT_EQ((size_t)1, map_->tunnels());
  EXPECT_EQ(null, map_->tunnel(tun0_->name()));
  EXPECT_EQ(null, map_->tunnelRemoteAddr(tun0_->remote()));

  // Delete the second tunnel, by name.
  map_->tunnelDel(tun1_->name());
  EXPECT_EQ((size_t)0, map_->tunnels());
  EXPECT_EQ(null, map_->tunnel(tun1_->name()));
  EXPECT_EQ(null, map_->tunnelRemoteAddr(tun1_->remote()));
}
