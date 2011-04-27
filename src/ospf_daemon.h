#ifndef OSPF_DAEMON_H_
#define OSPF_DAEMON_H_

#include "fwk/log.h"
#include "fwk/ptr.h"

#include "control_plane.h"
#include "ospf_router.h"
#include "task.h"


class OSPFDaemon : public PeriodicTask {
 public:
  typedef Fwk::Ptr<const OSPFDaemon> PtrConst;
  typedef Fwk::Ptr<OSPFDaemon> Ptr;

  static Ptr New(OSPFRouter::Ptr ospf_rtr, ControlPlane::Ptr cp) {
    return new OSPFDaemon(ospf_rtr, cp);
  }

 protected:
  OSPFDaemon(OSPFRouter::Ptr ospf_rtr, ControlPlane::Ptr cp);

  void run();

  ControlPlane::Ptr cp_;
  OSPFRouter::Ptr ospf_rtr_;
  Fwk::Log::Ptr log_;
};


#endif
