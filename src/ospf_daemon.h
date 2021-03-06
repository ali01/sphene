#ifndef OSPF_DAEMON_H_
#define OSPF_DAEMON_H_

#include <ctime>

#include "fwk/ptr.h"

#include "ospf_router.h"
#include "ospf_types.h"
#include "task.h"
#include "time_types.h"

/* Forward declarations. */
class OSPFInterface;
class OSPFNode;
class ControlPlane;
class DataPlane;


class OSPFDaemon : public PeriodicTask {
 public:
  typedef Fwk::Ptr<const OSPFDaemon> PtrConst;
  typedef Fwk::Ptr<OSPFDaemon> Ptr;

  static Ptr New(Fwk::Ptr<ControlPlane> cp, Fwk::Ptr<DataPlane> dp);

 protected:
  OSPFDaemon(Fwk::Ptr<ControlPlane> cp, Fwk::Ptr<DataPlane> dp);

  class RouterReactor : public OSPFRouter::Notifiee {
   public:
    typedef Fwk::Ptr<const RouterReactor> PtrConst;
    typedef Fwk::Ptr<RouterReactor> Ptr;

    static Ptr New(OSPFDaemon* daemon) {
      return new RouterReactor(daemon);
    }

    void onLinkStateFlood(OSPFRouter::Ptr _r) { daemon_->timeSinceLSUIs(0); }

   private:
    RouterReactor(OSPFDaemon* daemon) : daemon_(daemon) {}

    /* Data members. */
    OSPFDaemon* daemon_;

    /* Operations disallowed. */
    RouterReactor(const RouterReactor&);
    void operator=(const RouterReactor&);
  };

  /* Override. */
  void run();

  /* --  Private attributes -- */

  Seconds timeSinceLSU() const { return ::time(NULL) - latest_lsu_; }
  void timeSinceLSUIs(Seconds delta);

  /* -- OSPFDaemon private helper functions. -- */

  /* Removes topology entries that have not been confirmed by an LSU in
     more than LSU_TIMEOUT seconds.*/
  void timeout_topology_entries();

  /* Helper function for timeout_topology_entries(). Accomplishes its task
     for a single node. */
  void timeout_node_topology_entries(Fwk::Ptr<OSPFNode> node);

  /* Removes links to neighbors who haven't sent a HELLO packet in more
     than three times their advertised HELLOINT. */
  void timeout_neighbor_links();

  /* Helper function for timeout_neighbor_links(). Accomplishes its task
     for a single interface. */
  void timeout_interface_neighbor_links(Fwk::Ptr<OSPFInterface> iface);

  /* For all neighbors N_i, sends a HELLO packet to N_i if the last HELLO sent
     through the interface, J_i, that N_i is connected to, occurred more than
     J_i.HELLOINT seconds ago. */
  void broadcast_timed_hello();

  /* Sends a HELLO packet to all neighbors connected to interface IFACE. */
  void broadcast_hello_out_interface(Fwk::Ptr<OSPFInterface> iface);

  /* Triggers LSU floods to all directly connected neighbors. */
  void flood_timed_lsu();

  /* Data members. */
  Fwk::Ptr<ControlPlane> control_plane_;
  Fwk::Ptr<DataPlane> data_plane_;
  Fwk::Ptr<OSPFRouter> ospf_router_;

  /* Reactor to Router notifications. */
  RouterReactor::Ptr router_reactor_;

  time_t latest_lsu_;

  /* Operations disallowed. */
  OSPFDaemon(const OSPFDaemon&);
  void operator=(const OSPFDaemon&);
};

#endif
