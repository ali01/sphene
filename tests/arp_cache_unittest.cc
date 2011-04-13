#include "gtest/gtest.h"

#include "arp_cache.h"

class ARPCacheTest : public ::testing::Test {
 protected:
  ARPCacheTest() {
    arp_cache_ = ARPCache::New();
    ip_addr_ = "192.168.0.1";
    eth_addr_ = "C0:FF:EE:BA:BE:EE";
  }

  ARPCache::Ptr arp_cache_;
  IPv4Addr ip_addr_;
  EthernetAddr eth_addr_;
};

TEST_F(ARPCacheTest, construction) {
  ASSERT_EQ(arp_cache_->entries(), (size_t)0);
  ASSERT_EQ(arp_cache_->begin(), arp_cache_->end());

  ARPCache::Entry::Ptr entry = arp_cache_->entry(ip_addr_);
  ASSERT_TRUE(entry == NULL);
}


void
test_single_entry(ARPCache::Ptr cache, IPv4Addr ip, EthernetAddr eth) {
  EXPECT_EQ(cache->entries(), (size_t)1);
  EXPECT_NE(cache->begin(), cache->end());

  ARPCache::Entry::Ptr entry = cache->entry(ip);
  ASSERT_TRUE(entry != NULL);
  EXPECT_EQ(entry->ipAddr(), ip);
  EXPECT_EQ(entry->ethernetAddr(), eth);
}

TEST_F(ARPCacheTest, insert_delete) {
  ARPCache::Entry::Ptr entry = ARPCache::Entry::EntryNew(ip_addr_, eth_addr_);
  arp_cache_->entryIs(entry);
  test_single_entry(arp_cache_, ip_addr_, eth_addr_);

  arp_cache_->entryIs(entry);
  test_single_entry(arp_cache_, ip_addr_, eth_addr_);

  for (int i = 0; i < 2; ++i) {
    arp_cache_->entryDel(ip_addr_);
    EXPECT_EQ(arp_cache_->entries(), (size_t)0);
    EXPECT_EQ(arp_cache_->begin(), arp_cache_->end());
  }
}
