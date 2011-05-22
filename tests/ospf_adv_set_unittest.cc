#include "gtest/gtest.h"

#include "ospf_adv_set.h"

class OSPFAdvertisementSetTest : public ::testing::Test {
 protected:
  OSPFAdvertisementSetTest() {
    addr_1_ = (uint32_t)0x1;
    addr_2_ = (uint32_t)0x2;
    rid_1_ = (uint32_t)0xf1;
    rid_2_ = (uint32_t)0xf2;
  }

  OSPFAdvertisementSet advs_;
  IPv4Addr addr_1_;
  IPv4Addr addr_2_;
  RouterID rid_1_;
  RouterID rid_2_;
};

TEST_F(OSPFAdvertisementSetTest, lookup) {
  EXPECT_EQ(advs_.advertisements(), (size_t)0);

  advs_.advertisementIs(rid_1_, rid_2_, addr_1_, addr_2_);
  EXPECT_TRUE(advs_.contains(rid_1_, rid_2_, addr_1_, addr_2_));

  advs_.advertisementDel(rid_1_, rid_2_, addr_1_, addr_2_);
  EXPECT_FALSE(advs_.contains(rid_1_, rid_2_, addr_1_, addr_2_));
}
