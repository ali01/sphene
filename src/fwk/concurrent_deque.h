/** \file concurrent_deque.h
 * Thread-safe double-ended queue data structure.
 * \author Matt Sparks
 */
#ifndef __FWK__CONCURRENTDEQUE_H__
#define __FWK__CONCURRENTDEQUE_H__

#include <pthread.h>

#include "deque.h"

namespace Fwk {

template <typename T>
class ConcurrentDeque : public Fwk::PtrInterface<ConcurrentDeque<T> > {
 public:
  typedef Fwk::Ptr<ConcurrentDeque<T> const> PtrConst;
  typedef Fwk::Ptr<ConcurrentDeque<T> > Ptr;

  static Ptr concurrentDequeNew() { return new ConcurrentDeque(); }
  ConcurrentDeque() {
    pthread_cond_init(&cond_, NULL);
    pthread_mutex_init(&mutex_, NULL);
  }

  virtual ~ConcurrentDeque() {
    pthread_cond_destroy(&cond_);
    pthread_mutex_destroy(&mutex_);
  }

  /* accessors */
  size_t size() const { return deque_.size(); }
  bool empty() const { return deque_.empty(); }

  T element(uint32_t i) {
    ScopedLock lock(&mutex_);
    return deque_[i];
  }

  T front() {
    ScopedLock lock(&mutex_);
    return deque_.front();
  }

  T back() {
    ScopedLock lock(&mutex_);
    return deque_.back();
  }

  const T element(uint32_t i) const { return element(i); }
  const T front() const { return front(); }
  const T back() const { return back(); }

  /* mutators */
  void elementIs(uint32_t _i, const T& _e) {
    ScopedLock lock(&mutex_);
    deque_[_i] = _e;
  }

  void pushFront(const T& _e) {
    ScopedLock lock(&mutex_);
    deque_.pushFront(_e);
    pthread_cond_signal(&cond_);
  }

  void pushBack(const T& _e) {
    ScopedLock lock(&mutex_);
    deque_.pushBack(_e);
    pthread_cond_signal(&cond_);
  }

  T popFront() {
    ScopedLock lock(&mutex_);
    while (deque_.empty())
      pthread_cond_wait(&cond_, &mutex_);
    T e = deque_.front();
    deque_.popFront();
    return e;
  }

  T popBack() {
    ScopedLock lock(&mutex_);
    while (deque_.empty())
      pthread_cond_wait(&cond_, &mutex_);
    T e = deque_.back();
    deque_.popBack();
    return e;
  }

  void clear() {
    ScopedLock lock(&mutex_);
    deque_.clear();
  }

 protected:
  // TODO(ms): Move this to a separate file.
  class ScopedLock {
   public:
    ScopedLock(pthread_mutex_t* mutex) {
      mutex_ = mutex;
      pthread_mutex_lock(mutex_);
    }

    ~ScopedLock() {
      pthread_mutex_unlock(mutex_);
    }

   private:
    pthread_mutex_t* mutex_;
  };

  Deque<T> deque_;
  pthread_cond_t cond_;
  pthread_mutex_t mutex_;
};

}

#endif
