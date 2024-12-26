#include <atomic>
#include <cstdint>

class ExactCounter {
 private:
  std::atomic<int64_t> global_counter_{0};

 public:
  ExactCounter() {}

  int64_t update(int64_t amount) {
    return global_counter_.fetch_add(amount, std::memory_order_relaxed);
  }

  int64_t get() const {
    return global_counter_.load(std::memory_order_relaxed);
  }
};
