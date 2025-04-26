#include <atomic>
#include <iostream>
#include <thread>
#include <vector>

#include "futex_based_mutex.h"
#include "portable_mutex.h"

#ifndef NTHREADS
#define NTHREADS 5
#endif

#ifndef ELEMS_PER_THREAD
#define ELEMS_PER_THREAD 1000
#endif

#ifndef PORTABLE
#define PORTABLE 1  // Default to portable mutex if not defined
#endif

void benchmark1() {
  int64_t vnoprotect = 0;
  std::vector<std::thread> threads;
  threads.reserve(NTHREADS);
  for (int i = 0; i < NTHREADS; i++) {
    threads.emplace_back([&vnoprotect]() -> void {
      for (int i = 0; i < ELEMS_PER_THREAD; ++i) {
        vnoprotect += 1;
      }
    });
  }

  for (auto& thread : threads) {
    thread.join();
  }

  const int64_t expected = NTHREADS * ELEMS_PER_THREAD;
  std::cout << "Got: " << vnoprotect << "; expected: " << expected << std::endl;
}

void benchmark2() {
#if PORTABLE
  portable_mutex mutex;
#else
  futex_based_mutex mutex;
#endif

  int counter = 0;
  std::vector<std::thread> threads;
  threads.reserve(NTHREADS);
  for (int i = 0; i < NTHREADS; i++) {
    threads.emplace_back([&mutex, &counter]() -> void {
      for (int j = 0; j < ELEMS_PER_THREAD; j++) {
        mutex.lock();
        counter++;
        mutex.unlock();
      }
    });
  }
  for (auto& thread : threads) {
    thread.join();
  }
  const int expected = NTHREADS * ELEMS_PER_THREAD;
  std::cout << "Got: " << counter << "; expected: " << expected << std::endl;
}

int main() {
#if PORTABLE
  std::cout << "Testing portable mutex..." << std::endl;
#else
  std::cout << "Testing futex-based mutex..." << std::endl;
#endif

  std::cout << "Benchmark 1 (no mutex, incorrect): " << std::endl;
  benchmark1();

  std::cout << "Benchmark 2 (using mutex): " << std::endl;
  benchmark2();

  return 0;
}
