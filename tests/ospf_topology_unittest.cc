#include "gtest/gtest.h"

#include "ospf_node.h"
#include "ospf_topology.h"

class OSPFTopologyTest : public ::testing::Test {
 protected:
  OSPFTopologyTest() {
    root_node_id_ = 0xdeadbeef;
    root_node_ = OSPFNode::New(root_node_id_);
    topology_ = OSPFTopology::New(root_node_);
  }

  RouterID root_node_id_;
  OSPFNode::Ptr root_node_;
  OSPFTopology::Ptr topology_;
};

TEST_F(OSPFTopologyTest, basic) {
  RouterID nd_id = 0xcafebabe;
  OSPFNode::Ptr node = OSPFNode::New(nd_id);
  OSPFNode::Ptr temp;

  topology_->nodeIs(node);
  EXPECT_EQ(topology_->nodes(), (size_t)2);

  temp = topology_->node(nd_id);
  EXPECT_EQ(temp, node);

  temp = topology_->node(root_node_id_);
  EXPECT_EQ(temp, root_node_);
}
