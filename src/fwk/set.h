#ifndef SET_H_5VYMOD4E
#define SET_H_5VYMOD4E

#include <set>
#include <functional>
using std::set;

#include "ptr_interface.h"

namespace Fwk {

template <typename T,
          typename Compare=std::less<T>,
          typename Allocator=std::allocator<T> >
class Set : public Fwk::PtrInterface<Set<T,Compare,Allocator> > {
public:
  typedef Fwk::Ptr<const Set<T,Compare,Allocator> > PtrConst;
  typedef Fwk::Ptr<Set<T,Compare,Allocator> > Ptr;

  typedef typename set<T,Compare,Allocator>::iterator iterator;
  typedef typename set<T,Compare,Allocator>::reverse_iterator reverse_iterator;
  typedef typename set<T,Compare,Allocator>::const_iterator const_iterator;
  typedef typename set<T,Compare,Allocator>::const_reverse_iterator
                                             const_reverse_iterator;

  static Ptr New() { return new Set(); }
  Set() {}

  virtual ~Set() {}

  iterator begin() {
    return set_.begin();
  }

  iterator end() {
    return set_.end();
  }

  reverse_iterator rbegin() {
    return set_.rbegin();
  }

  reverse_iterator rend() {
    return set_.rend();
  }

  const_iterator begin() const {
    return set_.begin();
  }

  const_iterator end() const {
    return set_.end();
  }

  const_reverse_iterator rbegin() const {
    return set_.rbegin();
  }

  const_reverse_iterator rend() const {
    return set_.rend();
  }

  size_t size() const {
    return set_.size();
  }

  bool empty() const {
    return set_.empty();
  }

  iterator elem(const T& _v) {
    return set_.find(_v);
  }

  const_iterator elem(const T& _v) const {
    return set_.find(_v);
  }

  size_t count(const T& _v) const {
    return set_.count(_v);
  }

  void clear() {
    set_.clear();
  }

  void elemIs(const T& _v) {
    set_.insert(_v);
  }

  template <typename input_iterator>
  void insert(input_iterator first, input_iterator last) {
    set_.insert(first, last);
  }

  void elemDel(const T& _v) {
    set_.erase(_v);
  }

private:
  /* data members */
  set<T,Compare,Allocator> set_;

  /* operations disallowed */
  Set(const Set&);
  void operator=(const Set&);
};

} /* end of namespace Fwk */

#endif
