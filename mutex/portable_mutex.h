#ifndef PORTABLE_MUTEX_H
#define PORTABLE_MUTEX_H

#include <atomic>

// Version 3 of the mutes in Drepper's "Futexes are Tricky" paper
// Modified to use built-in functions instead of futexes
class portable_mutex {
 private:
  // An atomic_compare_exchange wrapper with semantics expected by the paper.
  static int cmpxchg(std::atomic<int>* val, int expected, int desired) {
    int* ep = &expected;
    std::atomic_compare_exchange_strong(val, ep, desired);
    return *ep;
  }

  enum {
    UNLOCKED,
    LOCKED,     // No waiters
    CONTENDED,  // There are waiters in lock()
  };

  // Can hold the values UNLOCKED, LOCKED, and CONTENDED
  std::atomic<int> val_;

 public:
  portable_mutex() : val_(UNLOCKED) {}

  void lock() {
    int status = cmpxchg(&val_, UNLOCKED, LOCKED);
    // We couldn't grab the lock, will have to wait...
    if (status != UNLOCKED) {
      // The lock is held by someone else. Signal that we are waiting by setting
      // the value to CONTENDED.
      if (status != CONTENDED) {
        status = val_.exchange(CONTENDED);
      }
      while (status != UNLOCKED) {
        // Wait until the lock is no longer CONTENDED.
        std::atomic_wait(&val_, CONTENDED);
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
      std::atomic_notify_one(&val_);
    }
  }

  bool try_lock() {
    int status = cmpxchg(&val_, UNLOCKED, LOCKED);
    return status == UNLOCKED;
  }
};

#endif  // PORTABLE_MUTEX_H
