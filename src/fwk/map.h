#ifndef MAP_H_XCYS69P4
#define MAP_H_XCYS69P4

#include <map>
#include <functional>
using std::map;

#include "ptr_interface.h"

/* Note:
 * Elements inserted into this map must inherit from PtrInterface.
 * Upon instantiation, ValueT template parameter must be specified without
 * the ::Ptr suffix -- namely,

     Fwk::Map<KeyT,ValueT> map_;

 * Not

     Fwk::Map<KeyT,ValueT::Ptr> map_;
 */

namespace Fwk {

template<typename KeyT,typename ValueT,typename Cmp=std::less<KeyT> >
class Map : public PtrInterface<Map<KeyT,ValueT,Cmp> > {
public:
  typedef Fwk::Ptr<const Map<KeyT,ValueT,Cmp> > PtrConst;
  typedef Fwk::Ptr<Map<KeyT,ValueT,Cmp> > Ptr;

  typedef typename map<KeyT,typename ValueT::Ptr,Cmp>::iterator iterator;
  typedef typename map<KeyT,typename ValueT::Ptr,Cmp>::reverse_iterator
    reverse_iterator;
  typedef typename map<KeyT,typename ValueT::Ptr,Cmp>::const_iterator
    const_iterator;
  typedef typename map<KeyT,typename ValueT::Ptr,Cmp>::const_reverse_iterator
    const_reverse_iterator;

  static Ptr New() { return new Map(); }
  static Ptr New(const Map& other) { return new Map(other); }

  Map() {}

  /* Shallow copy constructor. */
  Map(const Map& other) {
    for (const_iterator it = other.begin(); it != other.end(); ++it)
      elemIs(it->first, it->second);
  }

  virtual ~Map() {}

  iterator begin() {
    return map_.begin();
  }

  iterator end() {
    return map_.end();
  }

  reverse_iterator rbegin() {
    return map_.rbegin();
  }

  reverse_iterator rend() {
    return map_.rend();
  }

  const_iterator begin() const {
    return map_.begin();
  }

  const_iterator end() const {
    return map_.end();
  }

  const_reverse_iterator rbegin() const {
    return map_.rbegin();
  }

  const_reverse_iterator rend()   const {
    return map_.rend();
  }

  size_t size() const {
    return map_.size();
  }

  bool empty() const {
    return map_.empty();
  }

  typename ValueT::Ptr elem(const KeyT& _key) {
    typename ValueT::Ptr elem = NULL;
    const_iterator it = map_.find(_key);
    if (it != this->end())
      elem = it->second;

    return elem;
  }

  typename ValueT::PtrConst elem(const KeyT& _key) const {
    Map *self = const_cast<Map*>(this);
    return self->elem(_key);
  }

  void elemIs(const KeyT& _key, typename ValueT::Ptr _v) {
    map_[_key] = _v;
  }

  void elemDel(iterator _it) {
    map_.erase(_it);
  }

  void elemDel(const KeyT& _key) {
    map_.erase(_key);
  }

  void clear() {
    map_.clear();
  }

  typename ValueT::Ptr operator[](const KeyT& key) {
    return map_[key];
  }

  typename ValueT::PtrConst operator[](const KeyT& key) const {
    return map_[key];
  }

private:
  /* data members */
  map<KeyT, typename ValueT::Ptr, Cmp> map_;

  /* operations disallowed */
  void operator=(const Map&);
};

} /* end of namespace Fwk */


#endif
