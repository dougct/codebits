#ifndef MUTEX_H
#define MUTEX_H

#include <atomic>

#define UNLOCKED 0
#define LOCKED 1
#define CONTENDED 2

// An atomic_compare_exchange wrapper with semantics expected by the paper's
// mutex - return the old value stored in val.
int cmpxchg(std::atomic<int>* val, int expected, int desired) {
  int* ep = &expected;
  std::atomic_compare_exchange_strong(val, ep, desired);
  return *ep;
}

// Based on https://eli.thegreenplace.net/2018/basics-of-futexes/
// Modified to use built-in functions instead of futexes
class mutex {
 public:
  mutex() : val_(UNLOCKED) {}

  void lock() {
    int c = cmpxchg(&val_, UNLOCKED, LOCKED);
    // If the lock was previously unlocked, there's nothing else for us to do.
    // Otherwise, we'll probably have to wait.
    if (c != UNLOCKED) {
      do {
        // If the mutex is locked, we signal that we're waiting by setting the
        // atom to 2. A shortcut checks is it's 2 already and avoids the atomic
        // operation in this case.
        if (c == CONTENDED || cmpxchg(&val_, LOCKED, CONTENDED) != UNLOCKED) {
          // Here we have to actually sleep, because the mutex is actually
          // locked. Note that it's not necessary to loop around this syscall;
          // a spurious wakeup will do no harm since we only exit the do...while
          // loop when val_ is indeed 0.
          std::atomic_wait(&val_, CONTENDED);
        }
        // We're here when either:
        // (a) the mutex was in fact unlocked (by an intervening thread).
        // (b) we slept waiting for the atom and were awoken.
        //
        // So we try to lock the atom again. We set teh state to 2 because we
        // can't be certain there's no other thread at this exact point. So we
        // prefer to err on the safe side.
      } while ((c = cmpxchg(&val_, UNLOCKED, CONTENDED)) != UNLOCKED);
    }
  }

  void unlock() {
    if (val_.fetch_sub(1) != LOCKED) {
      val_.store(UNLOCKED);
      std::atomic_notify_one(&val_);
    }
  }

  bool try_lock() {
    int c = cmpxchg(&val_, UNLOCKED, LOCKED);
    return c == UNLOCKED;
  }

 private:
  // 0 means unlocked
  // 1 means locked, no waiters
  // 2 means locked, there are waiters in lock()
  std::atomic<int> val_;
};

/*
// Version 3 of Drepper's paper "Futexes are Tricky"
// Modified to use built-in functions instead of futexes

class mutex3 {
 public:
  mutex3() : val_(UNLOCKED) {}

  void lock() {
    int c;
    if ((c = cmpxchg(&val_, UNLOCKED, LOCKED)) != UNLOCKED) {
      if (c != CONTENDED) {
        c = val_.exchange(CONTENDED);
      }
      while (c != UNLOCKED) {
        std::atomic_wait(&val_, CONTENDED);
        c = val_.exchange(CONTENDED);
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
    int c = cmpxchg(&val_, UNLOCKED, LOCKED);
    return c == UNLOCKED;
  }

 private:
  // 0 means unlocked
  // 1 means locked, no waiters
  // 2 means locked, there are waiters in lock()
  std::atomic<int> val_;
};
*/

#endif  // MUTEX_H
