/** \file atomic.h
 * Atomic type templates.
 * \author Matt Sparks
 */
#ifndef __FWK__ATOMIC_H__
#define __FWK__ATOMIC_H__

#include <inttypes.h>
#include <ck_pr.h>


namespace Fwk {

template <class T>
class AtomicType32 {
 public:
  AtomicType32<T>(const T& v) : value_(v) { }
  AtomicType32<T>() { }

  T value() const {
    uint32_t v = ck_pr_load_32((uint32_t*)&value_);
    return *(T*)&v;
  }

  const T& operator=(const T& v) {
    ck_pr_store_32((uint32_t*)&value_, *(uint32_t*)&v);
    return v;
  }

  bool operator==(const T& other) const {
    T v = value();
    return (v == other);
  }

  bool cas(const T& compare, const T& update) {
    return ck_pr_cas_32((uint32_t*)&value_,
                        *(uint32_t*)&compare,
                        *(uint32_t*)&update);
  }

 protected:
  T value_;

  T* valuePtr() { return &value_; }
};


template <class T>
class AtomicTypeInt32 : public AtomicType32<T> {
 public:
  AtomicTypeInt32(const T& v) : AtomicType32<T>(v) { }
  AtomicTypeInt32() { }

  T operator+=(const T& other) {
    T* const value_ptr = AtomicType32<T>::valuePtr();
    ck_pr_add_32((uint32_t*)value_ptr, other);
    return AtomicType32<T>::value();
  }

  T operator-=(const T& other) {
    T* const value_ptr = AtomicType32<T>::valuePtr();
    ck_pr_sub_32((uint32_t*)value_ptr, other);
    return AtomicType32<T>::value();
  }

  T operator++() {     // prefix
    operator+=(1);
    return AtomicType32<T>::value();
  }

  T operator++(int) {  // postfix
    T v = AtomicType32<T>::value();
    operator++();
    return v;
  }

  T operator--() {     // prefix
    operator-=(1);
    return AtomicType32<T>::value();
  }

  T operator--(int) {  // postfix
    T v = AtomicType32<T>::value();
    operator--();
    return v;
  }
};


template <class T>
class AtomicType64 {
 public:
  AtomicType64<T>(const T& v) : value_(v) { }
  AtomicType64<T>() { }

  T value() const {
    uint64_t v = ck_pr_load_64((uint64_t*)&value_);
    return *(T*)&v;
  }

  const T& operator=(const T& v) {
    ck_pr_store_64((uint64_t*)&value_, *(uint64_t*)&v);
    return v;
  }

  bool operator==(const T& other) const {
    T v = value();
    return (v == other);
  }

  bool cas(const T& compare, const T& update) {
    return ck_pr_cas_64((uint64_t*)&value_,
                        *(uint64_t*)&compare,
                        *(uint64_t*)&update);
  }

 protected:
  T value_;

  T* valuePtr() { return &value_; }
};


template <class T>
class AtomicTypeInt64 : public AtomicType64<T> {
 public:
  AtomicTypeInt64(const T& v) : AtomicType64<T>(v) { }
  AtomicTypeInt64() { }

  T operator+=(const T& other) {
    T* const value_ptr = AtomicType64<T>::valuePtr();
    ck_pr_add_64((uint64_t*)value_ptr, other);
    return AtomicType64<T>::value();
  }

  T operator-=(const T& other) {
    T* const value_ptr = AtomicType64<T>::valuePtr();
    ck_pr_sub_64((uint64_t*)value_ptr, other);
    return AtomicType64<T>::value();
  }

  T operator++() {     // prefix
    operator+=(1);
    return AtomicType64<T>::value();
  }

  T operator++(int) {  // postfix
    T v = AtomicType64<T>::value();
    operator++();
    return v;
  }

  T operator--() {     // prefix
    operator-=(1);
    return AtomicType64<T>::value();
  }

  T operator--(int) {  // postfix
    T v = AtomicType64<T>::value();
    operator--();
    return v;
  }
};


typedef AtomicTypeInt32<int32_t> AtomicInt32;
typedef AtomicTypeInt32<uint32_t> AtomicUInt32;

typedef AtomicTypeInt64<int64_t> AtomicInt64;
typedef AtomicTypeInt64<uint64_t> AtomicUInt64;

}

#endif
