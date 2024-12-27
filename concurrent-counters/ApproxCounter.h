#pragma once

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <numeric>
#include <thread>
#include <vector>

class ApproxCounter {
 private:
  uint32_t threshold_;
  std::atomic<int64_t> global_counter_{0};
  std::vector<int64_t> local_counters_;
  std::vector<std::mutex> local_mutexes_;
  std::atomic<uint64_t> num_updates_{1}; // For consistency with threshold_

 public:
  explicit ApproxCounter(uint32_t threshold, uint32_t num_threads)
      : threshold_(threshold),
        local_counters_(num_threads),
        local_mutexes_(num_threads) {}

  int64_t update(int64_t amount) {
    // Round-robin updating local counters
    uint64_t curr_update = num_updates_.fetch_add(1, std::memory_order_seq_cst);

    {
      const uint64_t idx = curr_update % local_counters_.size();
      std::unique_lock<std::mutex> lock(local_mutexes_[idx]);
      local_counters_[idx] += amount;
    }

    if (curr_update >= threshold_) {
      num_updates_.store(0, std::memory_order_relaxed);
      for (size_t i = 0; i < local_counters_.size(); i++) {
        std::unique_lock<std::mutex> lock(local_mutexes_[i]);
        global_counter_.fetch_add(local_counters_[i], std::memory_order_seq_cst);
        local_counters_[i] = 0;
      }
    }

    return global_counter_.load(std::memory_order_relaxed);
  }

  int64_t get() const {
    return global_counter_.load(std::memory_order_relaxed);
  }

  int64_t collect() {
    for (size_t i = 0; i < local_counters_.size(); i++) {
      std::unique_lock<std::mutex> lock(local_mutexes_[i]);
      global_counter_.fetch_add(local_counters_[i], std::memory_order_seq_cst);
      local_counters_[i] = 0;
    }
    return global_counter_;
  }
};
