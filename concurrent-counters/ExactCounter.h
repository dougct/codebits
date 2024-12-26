#include <atomic>
#include <cstdint>

class ExactCounter {
 private:
  int64_t global_counter_{0};
  std::mutex global_counter_mutex_;

 public:
  ExactCounter() {}

  int64_t update(int64_t amount) {
    std::unique_lock<std::mutex> lock{global_counter_mutex_};
    global_counter_ += amount;
    return global_counter_;
  }

  int64_t get() {
    std::unique_lock<std::mutex> lock{global_counter_mutex_};
    return global_counter_;
  }
};
