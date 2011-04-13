#ifndef ARP_QUEUE_H_G44N5TIX
#define ARP_QUEUE_H_G44N5TIX

#include <map>

#include "fwk/ptr_interface.h"
#include "fwk/linked_list.h"
#include "ip_packet.h"

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

    static Ptr New(IPPacket::Ptr packet) {
      return new PacketWrapper(packet);
    }

    IPPacket::Ptr packet() const { return packet_; }

   protected:
    PacketWrapper(IPPacket::Ptr packet) : packet_(packet) {}

    /* Data members. */
    IPPacket::Ptr packet_;

    /* Operations disallowed. */
    PacketWrapper(const PacketWrapper&);
    void operator=(const PacketWrapper&);
  };

  /* Nested queue entry class */
  class Entry : public Fwk::PtrInterface<Entry> {
   public:
    typedef Fwk::Ptr<const Entry> PtrConst;
    typedef Fwk::Ptr<Entry> Ptr;

    static Ptr New(const IPv4Addr& ip) {
      return new Entry(ip);
    }

    const IPv4Addr& ipAddr() const { return ip_; }

    void packetIs(IPPacket::Ptr packet);
    PacketWrapper::Ptr front() const { return packet_queue_.front(); }

   protected:
    Entry(const IPv4Addr& ip) : ip_(ip) {}

   private:
    /* Data members. */
    IPv4Addr ip_;
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
