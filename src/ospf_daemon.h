#ifndef OSPF_DAEMON_H_
#define OSPF_DAEMON_H_

#include "fwk/ptr.h"

#include "task.h"

/* Forward declarations. */
class OSPFRouter;
class ControlPlane;


class OSPFDaemon : public PeriodicTask {
 public:
  typedef Fwk::Ptr<const OSPFDaemon> PtrConst;
  typedef Fwk::Ptr<OSPFDaemon> Ptr;

  static Ptr New(Fwk::Ptr<OSPFRouter> ospf_router, Fwk::Ptr<ControlPlane> cp);

 protected:
  OSPFDaemon(Fwk::Ptr<OSPFRouter> ospf_router, Fwk::Ptr<ControlPlane> cp);

  void run();

  /* Data members. */
  Fwk::Ptr<ControlPlane> cp_;
  Fwk::Ptr<OSPFRouter> ospf_router_;

  /* Operations disallowed. */
  OSPFDaemon(const OSPFDaemon&);
  void operator=(const OSPFDaemon&);
};

#endif
