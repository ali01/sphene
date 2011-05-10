#include "gtest/gtest.h"

#include "ospf_link.h"
#include "ospf_node.h"
#include "ospf_topology.h"

static const unsigned int kNodes = 10;

class OSPFTopologyTest : public ::testing::Test {
 protected:
  OSPFTopologyTest() {
    for (unsigned int i = 0; i < kNodes; ++i) {
      node_ids_[i] = i;
      nodes_[i] = OSPFNode::New(i);
      links_[i] = OSPFLink::New(nodes_[i], (uint32_t)0, (uint32_t)0);
    }

    root_node_id_ = 0xffffffff;
    root_node_ = OSPFNode::New(root_node_id_);
    root_link_ = OSPFLink::New(root_node_, (uint32_t)0, (uint32_t)0);
    topology_ = OSPFTopology::New(root_node_);
  }

  
  RouterID node_ids_[kNodes];
  OSPFNode::Ptr nodes_[kNodes];
  OSPFLink::Ptr links_[kNodes];

  RouterID root_node_id_;
  OSPFNode::Ptr root_node_;
  OSPFLink::Ptr root_link_;
  OSPFTopology::Ptr topology_;
};

TEST_F(OSPFTopologyTest, basic) {
  OSPFNode::Ptr temp;
  OSPFNode::Ptr node = nodes_[0];

  topology_->nodeIs(node);
  EXPECT_EQ(topology_->nodes(), (size_t)2);

  temp = topology_->node(node->routerID());
  EXPECT_EQ(temp, node);

  temp = topology_->node(root_node_id_);
  EXPECT_EQ(temp, root_node_);
}

TEST_F(OSPFTopologyTest, two_node) {
  topology_->nodeIs(nodes_[0]);
  root_node_->linkIs(links_[0]);

  EXPECT_EQ(nodes_[0]->prev(), root_node_);
  EXPECT_EQ(topology_->nextHop(node_ids_[0]), nodes_[0]);
}

TEST_F(OSPFTopologyTest, two_node_reverse) {
  nodes_[0]->linkIs(root_link_);
  topology_->nodeIs(nodes_[0]);

  EXPECT_EQ(nodes_[0]->prev(), root_node_);
  EXPECT_EQ(topology_->nextHop(node_ids_[0]), nodes_[0]);
}
