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
      ids_[i] = i + 1;
      nodes_[i] = OSPFNode::New(i + 1);
      links_[i] = OSPFLink::New(nodes_[i], (uint32_t)0, (uint32_t)0);
    }

    root_node_id_ = 0x00badcafe;
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
  topology_->onUpdate();
  EXPECT_EQ(topology_->nodes(), (size_t)2);

  temp = topology_->node(node->routerID());
  EXPECT_EQ(temp, node);

  temp = topology_->node(root_node_id_);
  EXPECT_EQ(temp, root_node_);
}

TEST_F(OSPFTopologyTest, two_node) {
  topology_->nodeIs(nodes_[0]);
  root_node_->linkIs(links_[0]);
  topology_->onUpdate();

  EXPECT_EQ(nodes_[0]->upstreamNode(), root_node_);
  EXPECT_EQ(topology_->nextHop(ids_[0]), nodes_[0]);
}

TEST_F(OSPFTopologyTest, two_node_reverse) {
  nodes_[0]->linkIs(root_link_);
  topology_->nodeIs(nodes_[0]);
  topology_->onUpdate();

  EXPECT_EQ(nodes_[0]->upstreamNode(), root_node_);
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
    topology_->nodeIs(nodes_[i]);

  root_node_->linkIs(links_[0]);
  root_node_->linkIs(links_[1]);
  root_node_->linkIs(links_[2]);

  nodes_[0]->linkIs(links_[1]);
  nodes_[1]->linkIs(links_[2]);
  nodes_[1]->linkIs(links_[3]);
  nodes_[1]->linkIs(links_[4]);

  topology_->onUpdate();
}

TEST_F(OSPFTopologyTest, six_node_1) {
  setup_six_node_topology();

  EXPECT_EQ(nodes_[0]->upstreamNode(), root_node_);
  EXPECT_EQ(nodes_[1]->upstreamNode(), root_node_);
  EXPECT_EQ(nodes_[2]->upstreamNode(), root_node_);
  EXPECT_EQ(nodes_[3]->upstreamNode(), nodes_[1]);
  EXPECT_EQ(nodes_[4]->upstreamNode(), nodes_[1]);

  EXPECT_EQ(topology_->nextHop(ids_[0]), nodes_[0]);
  EXPECT_EQ(topology_->nextHop(ids_[1]), nodes_[1]);
  EXPECT_EQ(topology_->nextHop(ids_[2]), nodes_[2]);
  EXPECT_EQ(topology_->nextHop(ids_[3]), nodes_[1]);
  EXPECT_EQ(topology_->nextHop(ids_[4]), nodes_[1]);
}

TEST_F(OSPFTopologyTest, six_node_2) {
  setup_six_node_topology();
  EXPECT_EQ(topology_->nodes(), (size_t)6);

  root_node_->activeLinkDel(ids_[1]);
  root_node_->activeLinkDel(ids_[2]);
  topology_->onUpdate();

  /* Topology:
              0        4
          ___/ \___   /
         /         \ /
        R           1
           ________/|
          /         |
        2           3
  */

  EXPECT_TRUE(root_node_->upstreamNode() == NULL);
  EXPECT_EQ(nodes_[0]->upstreamNode(), root_node_);
  EXPECT_EQ(nodes_[1]->upstreamNode(), nodes_[0]);
  EXPECT_EQ(nodes_[2]->upstreamNode(), nodes_[1]);
  EXPECT_EQ(nodes_[3]->upstreamNode(), nodes_[1]);
  EXPECT_EQ(nodes_[4]->upstreamNode(), nodes_[1]);

  for (int i = 0; i < 5; ++i)
    EXPECT_EQ(topology_->nextHop(ids_[i]), nodes_[0]);
}

TEST_F(OSPFTopologyTest, self_link) {
  nodes_[0]->linkIs(links_[0]);
  topology_->onUpdate();
  EXPECT_EQ(nodes_[0]->links(), (size_t)0);
}

TEST_F(OSPFTopologyTest, link_delete) {
  nodes_[0]->linkIs(links_[1]);
  topology_->onUpdate();
  EXPECT_EQ(nodes_[0]->links(), (size_t)1);
  EXPECT_EQ(nodes_[1]->links(), (size_t)1);

  for (int i = 0, j = 1; i < 2; ++i, j = (i + 1) % 2) {
    OSPFLink::Ptr link = nodes_[i]->activeLink(nodes_[j]->routerID());
    ASSERT_TRUE(link != NULL);
    EXPECT_EQ(link->node(), nodes_[j]);
  }

  nodes_[0]->activeLinkDel(nodes_[1]->routerID());
  topology_->onUpdate();
  EXPECT_EQ(nodes_[0]->links(), (size_t)0);
  EXPECT_EQ(nodes_[1]->links(), (size_t)0);
}

TEST_F(OSPFTopologyTest, link_delete_reverse) {
  nodes_[0]->linkIs(links_[1]);
  nodes_[0]->activeLinkDel(nodes_[1]->routerID());
  topology_->onUpdate();
  EXPECT_EQ(nodes_[0]->links(), (size_t)0);
  EXPECT_EQ(nodes_[1]->links(), (size_t)0);
}
