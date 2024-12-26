#pragma once

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <numeric>
#include <thread>
#include <vector>

#include <iostream>

class ApproxCounter {
 private:
  uint32_t threshold_;
  uint32_t num_threads_;
  std::atomic<uint64_t> num_updates_{0};
  std::atomic<int64_t> global_counter_{0};
  std::vector<int64_t> local_counters_;
  std::vector<std::mutex> local_counters_mutexes_;

 public:
  explicit ApproxCounter(uint32_t threshold, uint32_t num_threads)
      : threshold_(threshold),
        num_threads_(num_threads),
        local_counters_(num_threads),
        local_counters_mutexes_(num_threads) {}

  int64_t update(int64_t amount) {
    // Round-robin updating local counters
    const uint32_t idx = num_updates_++ % num_threads_;
    {
      std::unique_lock<std::mutex> lock(local_counters_mutexes_[idx]);
      local_counters_[idx] += amount;
      if (num_updates_ >= threshold_) {
        global_counter_.fetch_add(local_counters_[idx]);
        local_counters_[idx] = 0;
        num_updates_.store(0, std::memory_order_relaxed);
      }
    }
    return global_counter_.load(std::memory_order_relaxed);
  }

  int64_t get() const {
    return global_counter_.load(std::memory_order_relaxed);
  }

  int64_t collect() {
    int64_t sum = std::accumulate(local_counters_.begin(), local_counters_.end(), 0);
    global_counter_.fetch_add(sum);
    return global_counter_;
  }

};
