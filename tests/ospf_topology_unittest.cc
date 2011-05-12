#include "gtest/gtest.h"

#include <ostream>

#include "ospf_link.h"
#include "ospf_node.h"
#include "ospf_topology.h"

static const int kNodes = 10;

static std::ostream&
operator<<(std::ostream& os, OSPFNode::Ptr node) {
  if (node == NULL)
    os << "Node(NULL)";
  else
    os << "Node(" << node->routerID() << ")";

  return os;
}

class OSPFTopologyTest : public ::testing::Test {
 protected:
  OSPFTopologyTest() {
    for (int i = 0; i < kNodes; ++i) {
      ids_[i] = i;
      nodes_[i] = OSPFNode::New(i);
      links_[i] = OSPFLink::New(nodes_[i], (uint32_t)0, (uint32_t)0);
    }

    root_node_id_ = 0xffffffff;
    root_node_ = OSPFNode::New(root_node_id_);
    root_link_ = OSPFLink::New(root_node_, (uint32_t)0, (uint32_t)0);
    topology_ = OSPFTopology::New(root_node_);
  }

  void setup_six_node_topology();
  
  RouterID ids_[kNodes];
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
  EXPECT_EQ(topology_->nextHop(ids_[0]), nodes_[0]);
}

TEST_F(OSPFTopologyTest, two_node_reverse) {
  nodes_[0]->linkIs(root_link_);
  topology_->nodeIs(nodes_[0]);

  EXPECT_EQ(nodes_[0]->prev(), root_node_);
  EXPECT_EQ(topology_->nextHop(ids_[0]), nodes_[0]);
}

void
OSPFTopologyTest::setup_six_node_topology() {
  /* Topology:
              0        4
          ___/ \___   /
         /         \ /
        R --------- 1
        |  ________/|
        | /         |
        2           3
  */

  for (int i = 0; i < 5; ++i)
    topology_->nodeIs(nodes_[i], false);

  root_node_->linkIs(links_[0], false);
  root_node_->linkIs(links_[1], false);
  root_node_->linkIs(links_[2], false);

  nodes_[0]->linkIs(links_[1], false);
  nodes_[1]->linkIs(links_[2], false);
  nodes_[1]->linkIs(links_[3], false);
  nodes_[1]->linkIs(links_[4], false);

  topology_->onPossibleUpdate();
}

TEST_F(OSPFTopologyTest, six_node_1) {
  setup_six_node_topology();

  EXPECT_EQ(nodes_[0]->prev(), root_node_);
  EXPECT_EQ(nodes_[1]->prev(), root_node_);
  EXPECT_EQ(nodes_[2]->prev(), root_node_);
  EXPECT_EQ(nodes_[3]->prev(), nodes_[1]);
  EXPECT_EQ(nodes_[4]->prev(), nodes_[1]);

  EXPECT_EQ(topology_->nextHop(ids_[0]), nodes_[0]);
  EXPECT_EQ(topology_->nextHop(ids_[1]), nodes_[1]);
  EXPECT_EQ(topology_->nextHop(ids_[2]), nodes_[2]);
  EXPECT_EQ(topology_->nextHop(ids_[3]), nodes_[1]);
  EXPECT_EQ(topology_->nextHop(ids_[4]), nodes_[1]);
}

TEST_F(OSPFTopologyTest, six_node_2) {
  setup_six_node_topology();
  EXPECT_EQ(topology_->nodes(), (size_t)6);

  root_node_->linkDel(ids_[1]);
  root_node_->linkDel(ids_[2]);

  /* Topology:
              0        4
          ___/ \___   /
         /         \ /
        R           1
           ________/|
          /         |
        2           3
  */

  EXPECT_TRUE(root_node_->prev() == NULL);
  EXPECT_EQ(nodes_[0]->prev(), root_node_);
  EXPECT_EQ(nodes_[1]->prev(), nodes_[0]);
  EXPECT_EQ(nodes_[2]->prev(), nodes_[1]);
  EXPECT_EQ(nodes_[3]->prev(), nodes_[1]);
  EXPECT_EQ(nodes_[4]->prev(), nodes_[1]);

  for (int i = 0; i < 5; ++i)
    EXPECT_EQ(topology_->nextHop(ids_[i]), nodes_[0]);
}

TEST_F(OSPFTopologyTest, self_link) {
  nodes_[0]->linkIs(links_[0]);
  EXPECT_EQ(nodes_[0]->links(), (size_t)0);
}
