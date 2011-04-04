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

  static Ptr SWDataPlaneNew() {
    return new SWDataPlane();
  }

 protected:
  SWDataPlane();

  /* Operations disallowed. */
  SWDataPlane(const SWDataPlane&);
  void operator=(const SWDataPlane&);
};

#endif
