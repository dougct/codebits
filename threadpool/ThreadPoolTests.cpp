#include <cassert>
#include <thread>

#include "gtest/gtest.h"

#include "ThreadPool.h"

TEST(ThreadPool, SimpleThreadPoolTest) {
  EXPECT_TRUE(true);
}

int main(int argc, char* argv[]) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
