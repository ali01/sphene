#include "gtest/gtest.h"

#include "ospf_node.h"
#include "ospf_topology.h"

static const unsigned int kNodes = 10;

class OSPFTopologyTest : public ::testing::Test {
 protected:
  OSPFTopologyTest() {
    for (unsigned int i = 0; i < kNodes; ++i) {
      nodes_[i] = OSPFNode::New(i + 1);
    }

    root_node_id_ = 0xdeadbeef;
    root_node_ = OSPFNode::New(root_node_id_);
    topology_ = OSPFTopology::New(root_node_);
  }

  RouterID root_node_id_;
  OSPFNode::Ptr nodes_[kNodes];
  OSPFNode::Ptr root_node_;
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
