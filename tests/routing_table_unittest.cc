#include "gtest/gtest.h"

#include "routing_table.h"

class RoutingTableTest : public ::testing::Test {
 protected:
  RoutingTableTest()
      : routing_table_(RoutingTable::New()),
        entry_one_(RoutingTable::Entry::New()),
        entry_two_(RoutingTable::Entry::New()),
        entry_three_(RoutingTable::Entry::New()) {
    entry_one_->subnetIs("184.72.19.45", "255.255.255.0");
    entry_two_->subnetIs("202.18.81.45", "255.255.0.0");
    entry_three_->subnetIs("202.18.81.45", "255.0.0.0");
  }

  RoutingTable::Ptr routing_table_;
  RoutingTable::Entry::Ptr entry_one_;
  RoutingTable::Entry::Ptr entry_two_;
  RoutingTable::Entry::Ptr entry_three_;
};

TEST_F(RoutingTableTest, basic) {
  ASSERT_TRUE(routing_table_->lpm("192.168.0.1") == NULL);
}
