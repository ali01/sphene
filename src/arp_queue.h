#ifndef ARP_QUEUE_H_G44N5TIX
#define ARP_QUEUE_H_G44N5TIX

#include <map>

#include "fwk/ptr_interface.h"
#include "fwk/linked_list.h"

#include "ethernet_packet.h"
#include "ip_packet.h"
#include "interface.h"

class ARPQueue : public Fwk::PtrInterface<ARPQueue> {
 public:
  typedef Fwk::Ptr<const ARPQueue> PtrConst;
  typedef Fwk::Ptr<ARPQueue> Ptr;

  /* Wrapper class that allows Packet objects
   * to be inserted into Fwk::LinkedList */
  class PacketWrapper : public Fwk::LinkedList<PacketWrapper>::Node {
   public:
    typedef Fwk::Ptr<const PacketWrapper> PtrConst;
    typedef Fwk::Ptr<PacketWrapper> Ptr;

    static Ptr New(EthernetPacket::Ptr packet) {
      return new PacketWrapper(packet);
    }

    EthernetPacket::Ptr packet() const { return packet_; }

   protected:
    PacketWrapper(EthernetPacket::Ptr packet) : packet_(packet) {}

    /* Data members. */
    EthernetPacket::Ptr packet_;

    /* Operations disallowed. */
    PacketWrapper(const PacketWrapper&);
    void operator=(const PacketWrapper&);
  };

  /* Nested queue entry class */
  class Entry : public Fwk::PtrInterface<Entry> {
   public:
    typedef Fwk::Ptr<const Entry> PtrConst;
    typedef Fwk::Ptr<Entry> Ptr;

    static Ptr New(const IPv4Addr& ip,
                   Interface::PtrConst interface,
                   EthernetPacket::PtrConst request) {
      return new Entry(ip, interface, request);
    }

    const IPv4Addr& ipAddr() const { return ip_; }
    Interface::PtrConst interface() const { return interface_; }
    EthernetPacket::PtrConst request() const { return request_; }
    unsigned int retries() const { return retries_; }
    void retriesInc() { retries_ += 1; }

    PacketWrapper::Ptr front() const { return packet_queue_.front(); }

    void packetIs(EthernetPacket::Ptr packet);

   protected:
    Entry(const IPv4Addr& ip,
          Interface::PtrConst interface,
          EthernetPacket::PtrConst request)
        : ip_(ip), interface_(interface), request_(request), retries_(0) { }

   private:
    /* Data members. */
    IPv4Addr ip_;
    Interface::PtrConst interface_;
    EthernetPacket::PtrConst request_;
    unsigned int retries_;
    Fwk::LinkedList<PacketWrapper> packet_queue_;

    /* Operations disallowed. */
    Entry(const Entry&);
    void operator=(const Entry&);
  };

  typedef std::map<IPv4Addr,Entry::Ptr>::iterator iterator;
  typedef std::map<IPv4Addr,Entry::Ptr>::const_iterator const_iterator;

  static Ptr New() {
    return new ARPQueue();
  }

  Entry::Ptr entry(const IPv4Addr& ip) const;
  void entryIs(Entry::Ptr entry);
  void entryDel(const IPv4Addr& ip);
  void entryDel(Entry::Ptr entry);

  iterator begin() { return addr_map_.begin(); }
  iterator end() { return addr_map_.end(); }
  const_iterator begin() const { return addr_map_.begin(); }
  const_iterator end() const { return addr_map_.end(); }

 protected:
  ARPQueue() {}

  /* Data members. */
  std::map<IPv4Addr,Entry::Ptr> addr_map_;

  /* Disallowed operations. */
  ARPQueue(const ARPQueue&);
  void operator=(const ARPQueue&);
};

#endif
