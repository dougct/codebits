#ifndef FUTEX_BASED_MUTEX_H
#define FUTEX_BASED_MUTEX_H

#include "futex_wrapper.h"

// Version 3 of the mutes in Drepper's "Futexes are Tricky" paper
class futex_based_mutex {
 private:
  // An atomic_compare_exchange wrapper with semantics expected by the paper.
  static uint32_t cmpxchg(std::atomic<uint32_t>* val,
                          uint32_t expected,
                          uint32_t desired) {
    uint32_t* ep = &expected;
    std::atomic_compare_exchange_strong(val, ep, desired);
    return *ep;
  }

  enum {
    UNLOCKED,
    LOCKED,     // No waiters
    CONTENDED,  // There are waiters in lock()
  };

  // Can hold the values UNLOCKED, LOCKED, and CONTENDED
  std::atomic<uint32_t> val_;

 public:
  futex_based_mutex() : val_(UNLOCKED) {}

  void lock() {
    uint32_t status = cmpxchg(&val_, UNLOCKED, LOCKED);
    // We couldn't grab the lock, will have to wait...
    if (status != UNLOCKED) {
      // The lock is held by someone else. Signal that we are waiting by setting
      // the value to CONTENDED.
      if (status != CONTENDED) {
        status = val_.exchange(CONTENDED);
      }
      while (status != UNLOCKED) {
        // Wait until the lock is no longer CONTENDED.
        futex_wait((uint32_t*)&val_, CONTENDED);
        // Here we have two cases to consider:
        //   1. The lock is LOCKED. This means that no other thread but this one
        //   is waiting on the lock. In this case, we will signal that we are
        //   waiting by setting it to CONTENDED. We will stay in the while loop.
        //   2. The lock is UNLOCKED. We will now grab the lock and exit the
        //   while loop. We have two choices at this point. We can either set it
        //   to LOCKED or CONTENDED. Since we can't be certain there's no other
        //   thread at this exact point, we set the lock to CONTENDED to be on
        //   the safe side.
        status = val_.exchange(CONTENDED);
      }
    }
  }

  void unlock() {
    if (val_.fetch_sub(1) != LOCKED) {
      val_.store(UNLOCKED);
      futex_wake((uint32_t*)&val_, LOCKED);
    }
  }

  bool try_lock() {
    uint32_t status = cmpxchg(&val_, UNLOCKED, LOCKED);
    return status == UNLOCKED;
  }
};

#endif  // FUTEX_BASED_MUTEX_H
