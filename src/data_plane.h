#ifndef DATA_PLANE_H_6H2UAS6L
#define DATA_PLANE_H_6H2UAS6L

#include <string>

#include "fwk/log.h"
#include "fwk/named_interface.h"

#include "interface.h"
#include "interface_map.h"
#include "packet.h"

/* Forward declarations. */
class ARPPacket;
class EthernetPacket;
class ICMPPacket;
class IPPacket;
class UnknownPacket;


class DataPlane : public Fwk::NamedInterface {
 public:
  typedef Fwk::Ptr<const DataPlane> PtrConst;
  typedef Fwk::Ptr<DataPlane> Ptr;

  void packetNew(Fwk::Ptr<EthernetPacket> pkt);

  InterfaceMap::Ptr interfaceMap() const;

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

    void operator()(ARPPacket*);
    void operator()(EthernetPacket*);
    void operator()(ICMPPacket*);
    void operator()(IPPacket*);
    void operator()(UnknownPacket*);

   private:
    DataPlane* dp_;
    Fwk::Log::Ptr log_;
  };

  Fwk::Log::Ptr log_;
  PacketFunctor functor_;
  InterfaceMap::Ptr iface_map_;
};

#endif
