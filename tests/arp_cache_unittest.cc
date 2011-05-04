#include "gtest/gtest.h"

#include "arp_cache.h"
#include "fwk/ptr.h"


// Test reactor class that counts the number of entries in the cache.
class ARPCacheReactor : public ARPCache::Notifiee {
 public:
  typedef Fwk::Ptr<ARPCacheReactor> Ptr;
  ARPCacheReactor() : entries_(0) { }

  static Ptr New() { return new ARPCacheReactor(); }

  virtual void onEntry(ARPCache::Entry::Ptr entry) { ++entries_; }
  virtual void onEntryDel(ARPCache::Entry::Ptr entry) { --entries_; }
  size_t entries() { return entries_; }

 private:
  size_t entries_;
};


class ARPCacheTest : public ::testing::Test {
 protected:
  void SetUp() {
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


TEST_F(ARPCacheTest, insertDelete) {
  ARPCache::Entry::Ptr entry = ARPCache::Entry::New(ip_addr_, eth_addr_);
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


TEST_F(ARPCacheTest, maxEntries) {
  uint32_t ip = 0;

  // Fill cache with maximum number of entries.
  for (uint i = 0; i < ARPCache::kMaxEntries; ++i) {
    ARPCache::Entry::Ptr entry = ARPCache::Entry::New(++ip, eth_addr_);
    entry->ageIs(i);
    arp_cache_->entryIs(entry);
  }
  uint32_t oldest_entry_ip = ip;
  EXPECT_EQ(ARPCache::kMaxEntries, arp_cache_->entries());
  EXPECT_TRUE(arp_cache_->entry(oldest_entry_ip));

  // Add one more entry. The "oldest" entry should be evicted.
  ARPCache::Entry::Ptr entry = ARPCache::Entry::New(++ip, eth_addr_);
  entry->ageIs(0);
  arp_cache_->entryIs(entry);
  EXPECT_EQ(ARPCache::kMaxEntries, arp_cache_->entries());

  // New entry should exist.
  EXPECT_TRUE(arp_cache_->entry(ip));
  // Previous-oldest entry should be gone.
  EXPECT_FALSE(arp_cache_->entry(oldest_entry_ip));
}


TEST_F(ARPCacheTest, reactor) {
  // Create a reactor.
  ARPCacheReactor::Ptr reactor = ARPCacheReactor::New();
  reactor->notifierIs(arp_cache_);

  // Cache is initially empty.
  EXPECT_EQ((size_t)0, reactor->entries());

  // Fill cache with a few entries.
  uint32_t ip = 0;
  const uint num_entries = 5;
  for (uint i = 0; i < num_entries; ++i) {
    ARPCache::Entry::Ptr entry = ARPCache::Entry::New(++ip, eth_addr_);
    entry->ageIs(i);
    arp_cache_->entryIs(entry);

    // Ensure reactor gets called properly.
    EXPECT_EQ(arp_cache_->entries(), reactor->entries());
  }

  // Remove all entries.
  ARPCache::iterator it;
  for (it = arp_cache_->begin(); it != arp_cache_->end(); ++it) {
    ARPCache::Entry::Ptr entry = it->second;
    arp_cache_->entryDel(entry);
    EXPECT_EQ(arp_cache_->entries(), reactor->entries());
  }
}
