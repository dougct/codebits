#include <atomic>
#include <cassert>
#include <chrono>
#include <thread>
#include <vector>

#include "ThreadPool.h"
#include "gtest/gtest.h"

TEST(BasicThreadPool, SingleTask) {
  BasicThreadPool pool;
  std::atomic<bool> taskExecuted{false};

  pool.submit([&taskExecuted]() { taskExecuted = true; });

  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  EXPECT_TRUE(taskExecuted);
}

TEST(BasicThreadPool, MultipleTasks) {
  BasicThreadPool pool;
  std::atomic<int> counter{0};

  const int numTasks = 100;
  for (int i = 0; i < numTasks; ++i) {
    pool.submit([&counter]() { counter++; });
  }

  std::this_thread::sleep_for(std::chrono::milliseconds(500));
  EXPECT_EQ(counter, numTasks);
}

TEST(SimpleThreadPool, SingleTask) {
  SimpleThreadPool pool;
  std::atomic<bool> taskExecuted{false};

  pool.submit([&taskExecuted]() { taskExecuted = true; });

  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  EXPECT_TRUE(taskExecuted);
}

TEST(SimpleThreadPool, MultipleTasks) {
  SimpleThreadPool pool;
  std::atomic<int> counter{0};

  const int numTasks = 100;
  for (int i = 0; i < numTasks; ++i) {
    pool.submit([&counter]() { counter++; });
  }

  std::this_thread::sleep_for(std::chrono::milliseconds(500));
  EXPECT_EQ(counter, numTasks);
}

int main(int argc, char* argv[]) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
