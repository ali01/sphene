#include "gtest/gtest.h"

#include <ostream>
#include <string>

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
    routing_table_ = RoutingTable::New();
    eth0_ = RoutingTable::Entry::New();
    eth1_ = RoutingTable::Entry::New();
    eth2_ = RoutingTable::Entry::New();
    eth3_ = RoutingTable::Entry::New();
    eth4_ = RoutingTable::Entry::New();

    // Destination   Gateway        Mask               Iface
    // 0.0.0.0       172.24.74.17   0.0.0.0            eth0
    // 10.3.0.29     10.3.0.29      255.255.255.255    eth1
    // 10.3.0.31     10.3.0.31      255.255.255.255    eth2
    // 10.99.0.0     10.99.0.1      255.255.0.0        eth3
    // 10.99.1.0     10.99.1.1      255.255.255.0      eth4

    eth0_->subnetIs("0.0.0.0", "0.0.0.0");
    eth0_->gatewayIs("172.24.74.17");

    eth1_->subnetIs("10.3.0.29", "255.255.255.255");
    eth1_->gatewayIs("10.3.0.29");

    eth2_->subnetIs("10.3.0.31", "255.255.255.255");
    eth2_->gatewayIs("10.3.0.31");

    eth3_->subnetIs("10.99.0.0", "255.255.0.0");
    eth3_->gatewayIs("10.99.0.1");

    eth4_->subnetIs("10.99.1.0", "255.255.255.0");
    eth4_->gatewayIs("10.99.1.1");
  }

  RoutingTable::Ptr routing_table_;
  RoutingTable::Entry::Ptr eth0_;
  RoutingTable::Entry::Ptr eth1_;
  RoutingTable::Entry::Ptr eth2_;
  RoutingTable::Entry::Ptr eth3_;
  RoutingTable::Entry::Ptr eth4_;
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
  EXPECT_EQ(2, routing_table_->entries());

  // More specific route before deletion.
  entry = routing_table_->lpm("10.3.0.29");
  EXPECT_EQ(eth1_, entry);

  // Ensure default route is used after deletion of specific route.
  routing_table_->entryDel(eth1_);
  EXPECT_EQ(1, routing_table_->entries());
  entry = routing_table_->lpm("10.3.0.29");
  EXPECT_EQ(eth0_, entry);

  // Default route before deletion.
  entry = routing_table_->lpm("184.72.19.250");
  EXPECT_EQ(eth0_, entry);

  // No default route after deletion.
  routing_table_->entryDel(eth0_);
  EXPECT_EQ(0, routing_table_->entries());
  entry = routing_table_->lpm("184.72.19.250");
  EXPECT_EQ(NULL, entry.ptr());
}


TEST_F(RoutingTableTest, duplicate) {
  // Start with 0 entries.
  EXPECT_EQ(0, routing_table_->entries());

  // Add an entry.
  routing_table_->entryIs(eth0_);
  EXPECT_EQ(1, routing_table_->entries());

  // Try to add the same entry again.
  routing_table_->entryIs(eth0_);
  EXPECT_EQ(1, routing_table_->entries());

  // Duplicate the object with the same subnet and gateway.
  RoutingTable::Entry::Ptr eth0_dup = RoutingTable::Entry::New();
  eth0_dup->subnetIs(eth0_->subnet(), eth0_->subnetMask());
  eth0_dup->gatewayIs(eth0_->gateway());

  // Add it. The routing table should not grow.
  routing_table_->entryIs(eth0_dup);
  EXPECT_EQ(1, routing_table_->entries());

  // Expect the default route (eth0) to be unchanged.
  // TODO(ms): perhaps the routes should be replaced instead of unchanged?
  EXPECT_EQ(eth0_, routing_table_->lpm("184.72.19.250"));
}
