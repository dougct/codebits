#include <thread>
#include <vector>
#include <numeric>

#include <gtest/gtest.h>

#include "ApproxCounter.h"

TEST(ApproxCounterTest, BasicUpdate) {
  ApproxCounter counter(100, 4);  // updates_threshold=100, num_threads=4
  int64_t result = counter.update(1);
  EXPECT_EQ(result, 0);  // First update should return previous global count
  EXPECT_EQ(counter.get(), 0);
}

TEST(ApproxCounterTest, ThresholdTrigger) {
  ApproxCounter counter(10, 1);  // Small updates_threshold for testing
  for (int i = 0; i <= 10; i++) {
    counter.update(1);
  }

  EXPECT_EQ(counter.get(), 10);
}

TEST(ApproxCounterTest, MultiThreadedUpdates) {
  const int num_threads = 4;
  const int updates_threshold = 1000;
  ApproxCounter counter(updates_threshold, num_threads);

  std::vector<std::thread> threads;
  const int updates_per_thread = 10000;

  for (int i = 0; i < num_threads; i++) {
    threads.emplace_back([&counter]() {
      for (int j = 0; j < updates_per_thread; j++) {
        counter.update(1);
      }
    });
  }

  for (auto& thread : threads) {
    thread.join();
  }

  // Total updates should be num_threads * updates_per_thread
  // But due to approximate nature, we allow some deviation
  const int64_t expected = num_threads * updates_per_thread;
  const int64_t actual = counter.get();
  EXPECT_EQ(expected, counter.collect());
  EXPECT_LE(actual, expected);  // Allow 10% undercount
}

TEST(ApproxCounterTest, RoundRobinDistribution) {
    ApproxCounter counter(1000, 3);  // 3 threads
    // Update 6 times to test round-robin behavior
    for (int i = 0; i < 6; i++) {
        counter.update(1);
    }
    // Each local counter should have received 2 updates
    EXPECT_EQ(counter.get(), 0);  // No updates_threshold reached yet
    EXPECT_EQ(counter.collect(), 6);
}

TEST(ApproxCounterTest, LargeUpdates) {
    ApproxCounter counter(2, 2);
    counter.update(500);
    // Should trigger updates_threshold on first local counter
    counter.update(501);
    counter.update(500);
    EXPECT_EQ(counter.get(), 1001);  // Second batch should be counted
}

TEST(ApproxCounterTest, ConcurrentReads) {
    const int num_threads = 4;
    ApproxCounter counter(100, num_threads);

    std::vector<std::thread> threads;
    const int iterations = 1000;

    // Half threads update, half read
    for (int i = 0; i < num_threads; i++) {
        if (i % 2 == 0) {
            threads.emplace_back([&counter]() {
                for (int j = 0; j < iterations; j++) {
                    counter.update(1);
                }
            });
        } else {
            threads.emplace_back([&counter]() {
                for (int j = 0; j < iterations; j++) {
                    counter.get();
                }
            });
        }
    }

    for (auto& thread : threads) {
        thread.join();
    }

    // Test completes without crashes or hangs
    SUCCEED();
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
