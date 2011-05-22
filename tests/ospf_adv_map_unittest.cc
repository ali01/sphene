#include "gtest/gtest.h"

#include <inttypes.h>

#include "ospf_adv_map.h"
#include "ospf_link.h"
#include "ospf_node.h"

static const int kAdvs = 10;

class OSPFAdvertisementMapTest : public ::testing::Test {
 protected:
  OSPFAdvertisementMapTest() {
    for (int i = 0; i < kAdvs; ++i) {
      ids_[i] = i;
      nodes_[i] = OSPFNode::New(i);
    }

    for (int i = 0; i < kAdvs; ++i) {
      links_[i] = OSPFLink::New(nodes_[(i + 1) % kAdvs],
                                (uint32_t)i, (uint32_t)(i + 1));
      advs_[i] = OSPFAdvertisement::New(nodes_[i], links_[i]);
    }
  }

  OSPFAdvertisementMap adv_map_;

  RouterID ids_[kAdvs];
  OSPFNode::Ptr nodes_[kAdvs];
  OSPFLink::Ptr links_[kAdvs];
  OSPFAdvertisement::Ptr advs_[kAdvs];
};

TEST_F(OSPFAdvertisementMapTest, basic) {
  EXPECT_EQ(adv_map_.advertisements(), (size_t)0);

  adv_map_.advertisementIs(advs_[0]);
  EXPECT_EQ(adv_map_.advertisements(), (size_t)1);

  OSPFAdvertisement::Ptr adv =
    adv_map_.advertisement(ids_[0],
                           links_[0]->nodeRouterID(),
                           links_[0]->subnet(),
                           links_[0]->subnetMask());
  EXPECT_TRUE(adv == advs_[0]);

  adv_map_.advertisementDel(adv->sender()->routerID(),
                            adv->link()->nodeRouterID(),
                            adv->link()->subnet(),
                            adv->link()->subnetMask());
  EXPECT_EQ(adv_map_.advertisements(), (size_t)0);
}

TEST_F(OSPFAdvertisementMapTest, common_id) {
  OSPFLink::Ptr link_one = OSPFLink::New(nodes_[2],
                                         (uint32_t)0x0badcafe,
                                         (uint32_t)0xffffffff);
  OSPFAdvertisement::Ptr adv_one = OSPFAdvertisement::New(nodes_[1], link_one);
  adv_map_.advertisementIs(adv_one);

  OSPFLink::Ptr link_two = OSPFLink::New(nodes_[2],
                                         (uint32_t)0x0,
                                         (uint32_t)0x0);
  OSPFAdvertisement::Ptr adv_two = OSPFAdvertisement::New(nodes_[1], link_two);
  adv_map_.advertisementIs(adv_two);

  EXPECT_EQ(adv_map_.advertisements(), (size_t)2);

  OSPFAdvertisement::Ptr cur;
  cur = adv_map_.advertisement(adv_one->sender()->routerID(),
                               adv_one->link()->nodeRouterID(),
                               adv_one->link()->subnet(),
                               adv_one->link()->subnetMask());
  EXPECT_TRUE(adv_one == cur);

  cur = adv_map_.advertisement(adv_two->sender()->routerID(),
                               adv_two->link()->nodeRouterID(),
                               adv_two->link()->subnet(),
                               adv_two->link()->subnetMask());
  EXPECT_TRUE(adv_two == cur);

  adv_map_.advertisementDel(adv_one);
  EXPECT_EQ(adv_map_.advertisements(), (size_t)1);

  adv_map_.advertisementDel(adv_two);
  EXPECT_EQ(adv_map_.advertisements(), (size_t)0);
}
