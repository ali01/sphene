#include "gtest/gtest.h"

#include <ostream>
#include <string>

#include "interface.h"
#include "routing_table.h"

using std::string;


// Provide a stream operator so we can see entry values easily.
static std::ostream& operator<<(std::ostream& os,
                                RoutingTable::Entry::Ptr entry) {
  if (!entry) {
    os << "Entry(NULL)";
  } else {
    os << "Entry(" << (string)entry->subnet() << ", "
       << (string)entry->subnetMask() << ")";
  }

  return os;
}


class RoutingTableTest : public ::testing::Test {
 protected:
  void SetUp() {
    routing_table_ = RoutingTable::New(NULL);
    eth0_ = RoutingTable::Entry::New(RoutingTable::Entry::kStatic);
    eth1_ = RoutingTable::Entry::New(RoutingTable::Entry::kStatic);
    eth2_ = RoutingTable::Entry::New(RoutingTable::Entry::kStatic);
    eth3_ = RoutingTable::Entry::New(RoutingTable::Entry::kStatic);
    eth4_ = RoutingTable::Entry::New(RoutingTable::Entry::kStatic);
    if_eth0_ = Interface::InterfaceNew("eth0");
    if_eth1_ = Interface::InterfaceNew("eth1");
    if_eth2_ = Interface::InterfaceNew("eth2");
    if_eth3_ = Interface::InterfaceNew("eth3");
    if_eth4_ = Interface::InterfaceNew("eth4");

    // Destination   Gateway        Mask               Iface
    // 0.0.0.0       172.24.74.17   0.0.0.0            eth0
    // 10.3.0.29     10.3.0.29      255.255.255.255    eth1
    // 10.3.0.31     10.3.0.31      255.255.255.255    eth2
    // 10.99.0.0     10.99.0.1      255.255.0.0        eth3
    // 10.99.1.0     10.99.1.1      255.255.255.0      eth4

    eth0_->subnetIs("0.0.0.0", "0.0.0.0");
    eth0_->gatewayIs("172.24.74.17");
    eth0_->interfaceIs(if_eth0_);

    eth1_->subnetIs("10.3.0.29", "255.255.255.255");
    eth1_->gatewayIs("10.3.0.29");
    eth1_->interfaceIs(if_eth1_);

    eth2_->subnetIs("10.3.0.31", "255.255.255.255");
    eth2_->gatewayIs("10.3.0.31");
    eth2_->interfaceIs(if_eth2_);

    eth3_->subnetIs("10.99.0.0", "255.255.0.0");
    eth3_->gatewayIs("10.99.0.1");
    eth3_->interfaceIs(if_eth3_);

    eth4_->subnetIs("10.99.1.0", "255.255.255.0");
    eth4_->gatewayIs("10.99.1.1");
    eth4_->interfaceIs(if_eth4_);
  }

  RoutingTable::Ptr routing_table_;
  RoutingTable::Entry::Ptr eth0_;
  RoutingTable::Entry::Ptr eth1_;
  RoutingTable::Entry::Ptr eth2_;
  RoutingTable::Entry::Ptr eth3_;
  RoutingTable::Entry::Ptr eth4_;
  Interface::Ptr if_eth0_;
  Interface::Ptr if_eth1_;
  Interface::Ptr if_eth2_;
  Interface::Ptr if_eth3_;
  Interface::Ptr if_eth4_;
};


TEST_F(RoutingTableTest, empty) {
  // Ensure we have no routes.
  ASSERT_EQ(NULL, routing_table_->lpm("192.168.0.1").ptr());
}


TEST_F(RoutingTableTest, defaultRoute) {
  // Add a default route and ensure all traffic goes through it.
  routing_table_->entryIs(eth0_);
  RoutingTable::Entry::Ptr entry = routing_table_->lpm("202.18.49.17");
  EXPECT_EQ(eth0_, entry);
}


TEST_F(RoutingTableTest, lpm) {
  // Add all routes.
  routing_table_->entryIs(eth0_);
  routing_table_->entryIs(eth1_);
  routing_table_->entryIs(eth2_);
  routing_table_->entryIs(eth3_);
  routing_table_->entryIs(eth4_);

  // Check default route.
  RoutingTable::Entry::Ptr entry = routing_table_->lpm("202.18.49.17");
  EXPECT_EQ(eth0_.ptr(), entry.ptr());
  EXPECT_EQ(eth0_, entry);

  // Test routes to the individual IPs on eth1 and eth2.
  entry = routing_table_->lpm("10.3.0.29");
  EXPECT_EQ(eth1_, entry);
  entry = routing_table_->lpm("10.3.0.31");
  EXPECT_EQ(eth2_, entry);

  // Test routes to 10.99.0.0/16 and 10.99.1.0/24.
  entry = routing_table_->lpm("10.99.15.15");
  EXPECT_EQ(eth3_, entry);
  entry = routing_table_->lpm("10.99.1.49");
  EXPECT_EQ(eth4_, entry);
}


TEST_F(RoutingTableTest, deletion) {
  RoutingTable::Entry::Ptr entry;
  routing_table_->entryIs(eth0_);
  routing_table_->entryIs(eth1_);
  EXPECT_EQ((size_t)2, routing_table_->entries());

  // More specific route before deletion.
  entry = routing_table_->lpm("10.3.0.29");
  EXPECT_EQ(eth1_, entry);

  // Ensure default route is used if the specific route's interface is down.
  if_eth1_->enabledIs(false);
  entry = routing_table_->lpm("10.3.0.29");
  EXPECT_EQ(eth0_, entry);

  // Re-enable the interface. Make sure static route still exists.
  if_eth1_->enabledIs(true);
  entry = routing_table_->lpm("10.3.0.29");
  EXPECT_EQ(eth1_, entry);

  // Ensure default route is used after deletion of specific route.
  routing_table_->entryDel(eth1_);
  EXPECT_EQ((size_t)1, routing_table_->entries());
  entry = routing_table_->lpm("10.3.0.29");
  EXPECT_EQ(eth0_, entry);

  // Default route before deletion.
  entry = routing_table_->lpm("184.72.19.250");
  EXPECT_EQ(eth0_, entry);

  // No default route after deletion.
  routing_table_->entryDel(eth0_);
  EXPECT_EQ((size_t)0, routing_table_->entries());
  entry = routing_table_->lpm("184.72.19.250");
  EXPECT_EQ(NULL, entry.ptr());
}


TEST_F(RoutingTableTest, duplicate) {
  // Start with 0 entries.
  EXPECT_EQ((size_t)0, routing_table_->entries());

  // Add an entry.
  routing_table_->entryIs(eth0_);
  EXPECT_EQ((size_t)1, routing_table_->entries());

  // Try to add the same entry again.
  routing_table_->entryIs(eth0_);
  EXPECT_EQ((size_t)1, routing_table_->entries());

  // Duplicate the object with the same subnet and gateway.
  RoutingTable::Entry::Ptr eth0_dup =
    RoutingTable::Entry::New(RoutingTable::Entry::kStatic);
  eth0_dup->subnetIs(eth0_->subnet(), eth0_->subnetMask());
  eth0_dup->gatewayIs(eth0_->gateway());
  eth0_dup->interfaceIs(eth0_->interface());

  // Add it. The routing table should not grow.
  routing_table_->entryIs(eth0_dup);
  EXPECT_EQ((size_t)1, routing_table_->entries());

  // Expect the default route (eth0) to be unchanged.
  RoutingTable::Entry::Ptr lpm_entry = routing_table_->lpm("184.72.19.250");
  EXPECT_EQ(eth0_->subnet(), lpm_entry->subnet());
}


TEST_F(RoutingTableTest, updateRoute) {
  // Add the default route.
  routing_table_->entryIs(eth0_);

  // Make a new object with an updated gateway.
  RoutingTable::Entry::Ptr eth0_dup =
    RoutingTable::Entry::New(RoutingTable::Entry::kStatic);
  eth0_dup->subnetIs(eth0_->subnet(), eth0_->subnetMask());
  eth0_dup->gatewayIs("4.2.2.1");
  eth0_dup->interfaceIs(eth0_->interface());

  // Add the duplicated default route to update the existing default route.
  routing_table_->entryIs(eth0_dup);

  // Ensure the default route was changed.
  RoutingTable::Entry::Ptr entry = routing_table_->lpm("184.72.19.250");
  EXPECT_EQ(eth0_->subnet(), entry->subnet());
  EXPECT_EQ(eth0_dup->gateway(), entry->gateway());
}
