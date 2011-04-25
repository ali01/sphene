#ifndef OSPF_ROUTER_H_LFORNADU
#define OSPF_ROUTER_H_LFORNADU

#include "fwk/log.h"
#include "fwk/ptr_interface.h"

#include "interface.h"
#include "ospf_packet.h"


/* Forward declarations. */
class Interface;


class OSPFRouter : public Fwk::PtrInterface<OSPFRouter> {
 public:
  typedef Fwk::Ptr<const OSPFRouter> PtrConst;
  typedef Fwk::Ptr<OSPFRouter> Ptr;

  static Ptr New() {
    return new OSPFRouter();
  }

  void packetNew(Packet::Ptr pkt, Interface::PtrConst iface);

 protected:
  OSPFRouter();

 private:
  class PacketFunctor : public Packet::Functor {
   public:
    PacketFunctor(OSPFRouter* ospf_router);

    void operator()(OSPFPacket*, Fwk::Ptr<const Interface>);
    void operator()(OSPFHelloPacket*, Fwk::Ptr<const Interface>);
    void operator()(OSPFLSUAdvertisement*, Fwk::Ptr<const Interface>);
    void operator()(OSPFLSUPacket*, Fwk::Ptr<const Interface>);

   private:
    OSPFRouter* ospf_router_;
    Fwk::Log::Ptr log_;
  };

  /* data members */
  Fwk::Log::Ptr log_;
  PacketFunctor functor_;

  /* operations disallowed */
  OSPFRouter(const OSPFRouter&);
  void operator=(const OSPFRouter&);
};

#endif
