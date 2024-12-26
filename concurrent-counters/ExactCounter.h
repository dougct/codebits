#include <atomic>
#include <cstdint>

class ExactCounter {
 private:
  int64_t counter_{0};
  std::mutex mutex_;

 public:
  ExactCounter() {}

  int64_t update(int64_t amount) {
    std::unique_lock<std::mutex> lock{mutex_};
    counter_ += amount;
    return counter_;
  }

  int64_t get() {
    std::unique_lock<std::mutex> lock{mutex_};
    return counter_;
  }
};
