#ifndef OSPF_DAEMON_H_
#define OSPF_DAEMON_H_

#include "fwk/ptr.h"

#include "ospf_types.h"
#include "task.h"

/* Forward declarations. */
class OSPFInterface;
class OSPFRouter;
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

  /* Override. */
  void run();

  /* -- OSPFDaemon private member functions. -- */

  /* For all neighbors N_i, sends a HELLO packet to N_i if the last HELLO sent
     through the interface, J_i, that N_i is connected to, occurred more than
     J_i.HELLOINT seconds ago. */
  void broadcast_timed_hello();

  /* Sends a HELLO packet to all neighbors connected to interface IFACE. */
  void broadcast_hello_out_interface(Fwk::Ptr<OSPFInterface> iface);

  /* Data members. */
  Fwk::Ptr<ControlPlane> control_plane_;
  Fwk::Ptr<DataPlane> data_plane_;
  Fwk::Ptr<OSPFRouter> ospf_router_;

  /* Operations disallowed. */
  OSPFDaemon(const OSPFDaemon&);
  void operator=(const OSPFDaemon&);
};

#endif
