#ifndef SOFTWARE_DATA_PLANE_H_
#define SOFTWARE_DATA_PLANE_H_

#include "fwk/ptr_interface.h"

#include "packet.h"
#include "data_plane.h"

/* Forward declarations. */
class ARPPacket;
class EthernetPacket;
class ICMPPacket;
class IPPacket;


class SWDataPlane : public DataPlane {
 public:
  typedef Fwk::Ptr<const SWDataPlane> PtrConst;
  typedef Fwk::Ptr<SWDataPlane> Ptr;

  static Ptr SWDataPlaneNew(struct sr_instance *sr) {
    return new SWDataPlane(sr);
  }

  virtual void outputPacketNew(Fwk::Ptr<EthernetPacket> pkt,
                               Interface::PtrConst iface);

 protected:
  SWDataPlane(struct sr_instance *sr);

  /* Operations disallowed. */
  SWDataPlane(const SWDataPlane&);
  void operator=(const SWDataPlane&);

 private:
  Fwk::Log::Ptr log_;
};

#endif
