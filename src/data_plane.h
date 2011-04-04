#ifndef DATA_PLANE_H_6H2UAS6L
#define DATA_PLANE_H_6H2UAS6L

#include <string>

#include "fwk/log.h"
#include "fwk/named_interface.h"

#include "packet.h"

/* Forward declarations. */
class ARPPacket;
class EthernetPacket;
class ICMPPacket;
class IPPacket;


class DataPlane : public Fwk::NamedInterface {
 public:

 protected:
  DataPlane(const std::string& name);

  class PacketFunctor : public Packet::Functor {
   public:
    void operator()(ARPPacket*);
    void operator()(EthernetPacket*);
    void operator()(ICMPPacket*);
    void operator()(IPPacket*);
  };

  /* Operations disallowed. */
  DataPlane(const DataPlane&);
  void operator=(const DataPlane&);
};

#endif
