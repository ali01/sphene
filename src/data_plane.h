#ifndef DATA_PLANE_H_6H2UAS6L
#define DATA_PLANE_H_6H2UAS6L

#include "fwk/ptr_interface.h"

#include "packet.h"

/* Forward declarations. */
class ARPPacket;
class EthernetPacket;
class ICMPPacket;
class IPPacket;


class DataPlane : public Fwk::PtrInterface<DataPlane> {
 public:
  typedef Fwk::Ptr<const DataPlane> PtrConst;
  typedef Fwk::Ptr<DataPlane> Ptr;

  static Ptr DataPlaneNew() {
    return new DataPlane();
  }

 protected:
  DataPlane();

  class PacketFunctor : public Packet::Functor {
   public:
    void operator()(ARPPacket *);
    void operator()(EthernetPacket *);
    void operator()(ICMPPacket *);
    void operator()(IPPacket *);
  };

  /* Operations disallowed. */
  DataPlane(const DataPlane&);
  void operator=(const DataPlane&);
};

#endif
