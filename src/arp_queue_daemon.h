#ifndef ARP_QUEUE_DAEMON_H_
#define ARP_QUEUE_DAEMON_H_

#include "fwk/log.h"
#include "fwk/ptr.h"

#include "arp_queue.h"
#include "control_plane.h"
#include "task.h"


class ARPQueueDaemon : public PeriodicTask {
 public:
  typedef Fwk::Ptr<const ARPQueueDaemon> PtrConst;
  typedef Fwk::Ptr<ARPQueueDaemon> Ptr;

  static Ptr New(ControlPlane::Ptr cp) { return new ARPQueueDaemon(cp); }

 protected:
  ARPQueueDaemon(ControlPlane::Ptr cp);

  void run();

  ControlPlane::Ptr cp_;
  Fwk::Log::Ptr log_;
};


#endif
