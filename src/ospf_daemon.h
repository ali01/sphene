#ifndef OSPF_DAEMON_H_
#define OSPF_DAEMON_H_

#include "fwk/ptr.h"

#include "task.h"

/* Forward declarations. */
class ControlPlane;
class DataPlane;
class OSPFRouter;


class OSPFDaemon : public PeriodicTask {
 public:
  typedef Fwk::Ptr<const OSPFDaemon> PtrConst;
  typedef Fwk::Ptr<OSPFDaemon> Ptr;

  static Ptr New(Fwk::Ptr<OSPFRouter> ospf_router,
                 Fwk::Ptr<ControlPlane> cp,
                 Fwk::Ptr<DataPlane> dp);

 protected:
  OSPFDaemon(Fwk::Ptr<OSPFRouter> ospf_router,
             Fwk::Ptr<ControlPlane> cp,
             Fwk::Ptr<DataPlane> dp);

  void run();

  /* Data members. */
  Fwk::Ptr<ControlPlane> control_plane_;
  Fwk::Ptr<DataPlane> data_plane_;
  Fwk::Ptr<OSPFRouter> ospf_router_;

  /* Operations disallowed. */
  OSPFDaemon(const OSPFDaemon&);
  void operator=(const OSPFDaemon&);
};

#endif
