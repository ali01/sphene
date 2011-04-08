#ifndef DATA_PLANE_H_6H2UAS6L
#define DATA_PLANE_H_6H2UAS6L

#include <string>

#include "fwk/log.h"
#include "fwk/named_interface.h"
#include "fwk/ptr.h"
#include "interface.h"
#include "interface_map.h"
#include "packet.h"

/* Forward declarations. */
class ARPPacket;
class ControlPlane;
class EthernetPacket;
class ICMPPacket;
class IPPacket;
class UnknownPacket;


class DataPlane : public Fwk::NamedInterface {
 public:
  typedef Fwk::Ptr<const DataPlane> PtrConst;
  typedef Fwk::Ptr<DataPlane> Ptr;

  void packetNew(Fwk::Ptr<EthernetPacket> pkt, Interface::PtrConst iface);

  InterfaceMap::Ptr interfaceMap() const;

  // Returns the ControlPlane.
  ControlPlane* controlPlane() const { return cp_; }

  // Sets the ControlPlane.
  void controlPlaneIs(ControlPlane* cp) {
    cp_ = cp;
    functor_.controlPlaneIs(cp);
  }

 protected:
  DataPlane(const std::string& name);
  virtual ~DataPlane() {}

  /* Operations disallowed. */
  DataPlane(const DataPlane&);
  void operator=(const DataPlane&);

 private:
  class PacketFunctor : public Packet::Functor {
   public:
    PacketFunctor(DataPlane* dp);

    void operator()(ARPPacket*, Interface::PtrConst);
    void operator()(EthernetPacket*, Interface::PtrConst);
    void operator()(ICMPPacket*, Interface::PtrConst);
    void operator()(IPPacket*, Interface::PtrConst);
    void operator()(UnknownPacket*, Interface::PtrConst);

    // Sets the ControlPlane.
    void controlPlaneIs(ControlPlane* cp) { cp_ = cp; }

   private:
    ControlPlane* cp_;
    DataPlane* dp_;
    Fwk::Log::Ptr log_;
  };

  Fwk::Log::Ptr log_;
  PacketFunctor functor_;
  InterfaceMap::Ptr iface_map_;
  ControlPlane* cp_;
};

#endif
