#include "gtest/gtest.h"

#include <string>
#include "ethernet_packet.h"
#include "interface.h"
#include "interface_map.h"
#include "ip_packet.h"

using std::string;


// Test reactor class that counts the number of entries in the map.
class InterfaceMapReactor : public InterfaceMap::Notifiee {
 public:
  typedef Fwk::Ptr<InterfaceMapReactor> Ptr;
  InterfaceMapReactor() : count_(0) { }

  static Ptr New() { return new InterfaceMapReactor(); }

  virtual void onInterface(Interface::Ptr iface) { ++count_; }
  virtual void onInterfaceDel(Interface::Ptr iface) { --count_; }
  size_t interfaces() { return count_; }

 private:
  size_t count_;
};


class InterfaceMapTest : public ::testing::Test {
 protected:
  virtual void SetUp() {
    map_ = InterfaceMap::InterfaceMapNew();
    eth0_ = Interface::InterfaceNew("eth0");
    eth0_->ipIs("8.8.4.4");

    eth1_ = Interface::InterfaceNew("eth1");
    eth1_->ipIs("4.2.2.1");
  }

  Interface::Ptr eth0_;
  Interface::Ptr eth1_;
  InterfaceMap::Ptr map_;
};


TEST_F(InterfaceMapTest, Insert) {
  // Map is initially empty.
  EXPECT_EQ((size_t)0, map_->interfaces());

  // Insert interfaces into the map.
  map_->interfaceIs(eth0_);
  map_->interfaceIs(eth1_);

  // Ensure the map size has increased.
  EXPECT_EQ((size_t)2, map_->interfaces());
}


TEST_F(InterfaceMapTest, Retrieve) {
  // Insert interfaces.
  map_->interfaceIs(eth0_);
  map_->interfaceIs(eth1_);

  // We get const pointers back from the map.
  Interface::PtrConst eth0(eth0_);
  Interface::PtrConst eth1(eth1_);

  // Ensure we can retrieve them by name.
  EXPECT_EQ(eth0, map_->interface(eth0->name()));
  EXPECT_EQ(eth1, map_->interface(eth1->name()));

  // Ensure we can retrieve them by IP address.
  EXPECT_EQ(eth0, map_->interfaceAddr(eth0_->ip()));
  EXPECT_EQ(eth1, map_->interfaceAddr(eth1_->ip()));

  // NULL pointers are returned for non-existent interfaces.
  Interface::PtrConst null(NULL);
  EXPECT_EQ(null, map_->interface("bogusiface"));
}


TEST_F(InterfaceMapTest, Delete) {
  Interface::PtrConst null(NULL);

  // Insert interfaces.
  map_->interfaceIs(eth0_);
  map_->interfaceIs(eth1_);
  ASSERT_EQ((size_t)2, map_->interfaces());

  // Delete one interface from the map.
  map_->interfaceDel(eth0_);
  EXPECT_EQ((size_t)1, map_->interfaces());
  EXPECT_EQ(null, map_->interface(eth0_->name()));
  EXPECT_EQ(null, map_->interfaceAddr(eth0_->ip()));

  // Delete the second tunnel, by name.
  map_->interfaceDel(eth1_->name());
  EXPECT_EQ((size_t)0, map_->interfaces());
  EXPECT_EQ(null, map_->interface(eth1_->name()));
  EXPECT_EQ(null, map_->interfaceAddr(eth1_->ip()));
}


TEST_F(InterfaceMapTest, reactor) {
  // Create a reactor.
  InterfaceMapReactor::Ptr reactor = InterfaceMapReactor::New();
  reactor->notifierIs(map_);

  // Map is initially empty.
  EXPECT_EQ((size_t)0, reactor->interfaces());

  // Insert interfaces.
  map_->interfaceIs(eth0_);
  EXPECT_EQ((size_t)1, reactor->interfaces());
  map_->interfaceIs(eth1_);
  EXPECT_EQ((size_t)2, reactor->interfaces());

  // Check that the reactor is called only when a *new* interface is added.
  map_->interfaceIs(eth0_);
  map_->interfaceIs(eth1_);
  EXPECT_EQ((size_t)2, reactor->interfaces());

  // Remove interfaces.
  map_->interfaceDel(eth0_);
  EXPECT_EQ((size_t)1, reactor->interfaces());
  map_->interfaceDel(eth1_);
  EXPECT_EQ((size_t)0, reactor->interfaces());
  map_->interfaceDel(eth1_);
  EXPECT_EQ((size_t)0, reactor->interfaces());
}
