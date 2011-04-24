#ifndef ROUTER_H_
#define ROUTER_H_

#include <string>

#include "control_plane.h"
#include "data_plane.h"
#include "fwk/log.h"
#include "fwk/named_interface.h"
#include "fwk/ptr.h"
#include "task.h"


class Router : public Fwk::NamedInterface {
 public:
  typedef Fwk::Ptr<const Router> PtrConst;
  typedef Fwk::Ptr<Router> Ptr;

  static Ptr New(const std::string& name,
                 ControlPlane::Ptr cp,
                 DataPlane::Ptr dp,
                 TaskManager::Ptr tm) {
    return new Router(name, cp, dp, tm);
  }

  ControlPlane::Ptr controlPlane() const { return cp_; }
  DataPlane::Ptr dataPlane() const { return dp_; }
  TaskManager::Ptr taskManager() const { return tm_; }

 protected:
  Router(const std::string& name,
         ControlPlane::Ptr cp,
         DataPlane::Ptr dp,
         TaskManager::Ptr tm);
  ~Router();

  ControlPlane::Ptr cp_;
  DataPlane::Ptr dp_;
  Fwk::Log::Ptr log_;
  TaskManager::Ptr tm_;
};

#endif
