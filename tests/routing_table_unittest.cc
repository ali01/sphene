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

  routing_table_->entryIs(entry_one_);
  routing_table_->entryIs(entry_two_);
  routing_table_->entryIs(entry_three_);

  RoutingTable::Entry::Ptr entry = routing_table_->lpm("202.18.49.17");
  ASSERT_TRUE(entry != NULL);
  EXPECT_EQ(entry->subnet(), IPv4Addr("202.18.0.0"));

  entry = routing_table_->lpm("202.49.29.19");
  ASSERT_TRUE(entry != NULL);
  EXPECT_EQ(entry->subnet(), IPv4Addr("202.0.0.0"));

  entry = routing_table_->lpm("184.72.19.250");
  ASSERT_TRUE(entry != NULL);
  EXPECT_EQ(entry->subnet(), IPv4Addr("184.72.19.0"));
}

TEST_F(RoutingTableTest, deletion) {
  RoutingTable::Entry::Ptr entry;
  routing_table_->entryIs(entry_one_);
  routing_table_->entryIs(entry_two_);
  routing_table_->entryIs(entry_three_);

  routing_table_->entryDel(entry_one_);
  entry = routing_table_->lpm("184.72.19.250");
  EXPECT_TRUE(entry == NULL);

  routing_table_->entryDel(entry_two_);
  entry = routing_table_->lpm("202.18.81.49");
  ASSERT_TRUE(entry != NULL);
  EXPECT_EQ(entry->subnet(), IPv4Addr("202.0.0.0"));
}
