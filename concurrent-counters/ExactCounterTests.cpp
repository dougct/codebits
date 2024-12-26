#include <atomic>
#include <thread>
#include <vector>

#include <gtest/gtest.h>

#include "ExactCounter.h"

class ExactCounterTest : public ::testing::Test {
 protected:
  ExactCounter counter;
};

TEST_F(ExactCounterTest, InitialValueIsZero) {
  EXPECT_EQ(counter.get(), 0);
}

TEST_F(ExactCounterTest, SingleUpdate) {
  counter.update(1);
  EXPECT_EQ(counter.get(), 1);
}

TEST_F(ExactCounterTest, MultipleUpdates) {
  counter.update(1);
  counter.update(2);
  counter.update(3);
  EXPECT_EQ(counter.get(), 6);
}

TEST_F(ExactCounterTest, NegativeUpdates) {
  counter.update(5);
  counter.update(-3);
  EXPECT_EQ(counter.get(), 2);
}

TEST_F(ExactCounterTest, ConcurrentUpdates) {
  const int num_threads = 4;
  const int updates_per_thread = 10000;
  const int64_t increment = 1;

  std::vector<std::thread> threads;

  for (int i = 0; i < num_threads; ++i) {
    threads.emplace_back([this]() {
      for (int j = 0; j < updates_per_thread; ++j) {
        counter.update(increment);
      }
    });
  }

  for (auto& thread : threads) {
    thread.join();
  }

  EXPECT_EQ(counter.get(), num_threads * updates_per_thread);
}

TEST_F(ExactCounterTest, ConcurrentIncrementAndDecrement) {
  const int num_threads = 4;
  const int updates_per_thread = 10000;

  std::vector<std::thread> threads;

  for (int i = 0; i < num_threads; ++i) {
    threads.emplace_back([this, i]() {
      int64_t increment = (i % 2 == 0) ? 1 : -1;
      for (int j = 0; j < updates_per_thread; ++j) {
        counter.update(increment);
      }
    });
  }

  for (auto& thread : threads) {
    thread.join();
  }

  EXPECT_EQ(counter.get(), 0);
}

TEST_F(ExactCounterTest, StressTest) {
  const int num_threads = 8;
  const int updates_per_thread = 100000;
  const std::vector<int64_t> increments = {1, -1, 2, -2, 5, -5};

  std::vector<std::thread> threads;
  std::atomic<int64_t> expected_sum{0};

  // Create threads that update the counter with different values
  for (int i = 0; i < num_threads; ++i) {
    threads.emplace_back([this, &increments, &expected_sum, i]() {
      int64_t increment = increments[i % increments.size()];
      for (int j = 0; j < updates_per_thread; ++j) {
        counter.update(increment);
        expected_sum.fetch_add(increment, std::memory_order_relaxed);
      }
    });
  }

  for (auto& thread : threads) {
    thread.join();
  }

  // Check final count matches expected sum
  EXPECT_EQ(counter.get(), expected_sum.load());
}

int main(int argc, char** argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
