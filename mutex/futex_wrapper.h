#include <atomic>
#include <cassert>
#include <chrono>
#include <cstdint>
#include <limits>
#include <type_traits>

#ifdef __linux__

#include <linux/futex.h>
#include <sys/syscall.h>
#include <unistd.h>

// libc++ uses SYS_futex as a universal syscall name. However, on 32 bit
// architectures with a 64 bit time_t, we need to specify SYS_futex_time64.
#if !defined(SYS_futex) && defined(SYS_futex_time64)
#define SYS_futex SYS_futex_time64
#endif

#elif defined(__FreeBSD__)

#include <sys/types.h>
#include <sys/umtx.h>

#else  // <- Add other operating systems here

// Baseline needs no new headers

#endif

/**
 * Futex is an atomic 32 bit unsigned integer that provides access to the
 * futex() syscall on that value.  It is templated in such a way that it
 * can interact properly with DeterministicSchedule testing.
 *
 * If you don't know how to use futex(), you probably shouldn't be using
 * this class.  Even if you do know how, you should have a good reason
 * (and benchmarks to back you up).
 *
 * Because of the semantics of the futex syscall, the futex family of
 * functions are available as free functions rather than member functions
 */

#ifdef __linux__

int futex_wait(uint32_t* uaddr, uint32_t val) {
  static constexpr timespec timeout = {2, 0};
  return syscall(SYS_futex, ptr, FUTEX_WAIT_PRIVATE, val, &timeout, 0, 0);
}

int futex_wake(uint32_t* uaddr, int val) {
  return syscall(SYS_futex, uaddr, FUTEX_WAKE, val, 0, 0, 0);
}

int futex_wake(uint32_t* uaddr, bool notify_one) {
  syscall(SYS_futex, ptr, FUTEX_WAKE_PRIVATE, notify_one ? 1 : INT_MAX, 0, 0,
          0);
}

#elif defined(__APPLE__)

extern "C" int __ulock_wait(
    uint32_t operation,
    void* addr,
    uint64_t value,
    uint32_t timeout); /* timeout is specified in microseconds */
extern "C" int __ulock_wake(uint32_t operation,
                            void* addr,
                            uint64_t wake_value);

#define UL_COMPARE_AND_WAIT 1
#define ULF_WAKE_ALL 0x00000100

int futex_wait(uint32_t* uaddr, uint32_t val) {
  return __ulock_wait(UL_COMPARE_AND_WAIT, uaddr, val, 0);
}

int futex_wake(uint32_t* uaddr, int val) {
  return __ulock_wake(UL_COMPARE_AND_WAIT, uaddr, val);
}

int futex_wake(uint32_t* uaddr, bool notify_one) {
  return __ulock_wake(UL_COMPARE_AND_WAIT | (notify_one ? 0 : ULF_WAKE_ALL),
                      uaddr, 0);
}

#else  // <- Add other operating systems here

#endif
