#include "gtest/gtest.h"

#include "SingleThreadedRingBuffer.h"

TEST(SingleThreadedRingBuffer, SingleThreadSimpleTest) {
  int numItems = 10;
  SingleThreadedRingBuffer<int> ring(numItems + 1);
  EXPECT_TRUE(ring.empty());
  EXPECT_TRUE(ring.push(1));
  EXPECT_EQ(*ring.front(), 1);
  int value;
  EXPECT_TRUE(ring.pop(value));
  EXPECT_EQ(value, 1);
  EXPECT_TRUE(ring.empty());
}

TEST(SingleThreadedRingBuffer, SingleThreadPopulateTest) {
  int numItems = 10;
  SingleThreadedRingBuffer<int> ring(numItems + 1);
  for (int i = 0; i < numItems; i++) {
    EXPECT_TRUE(ring.push(i));
  }
  EXPECT_TRUE(ring.full());
  EXPECT_FALSE(ring.push(0));

  for (int i = 0; i < numItems; i++) {
    int value;
    EXPECT_TRUE(ring.pop(value));
    EXPECT_EQ(value, i);
  }
  EXPECT_TRUE(ring.empty());
}

TEST(SingleThreadedRingBuffer, SingleThreadEmptyTest) {
  int numItems = 10;
  SingleThreadedRingBuffer<int> ring(numItems + 1);
  for (int i = 0; i < numItems; i++) {
    EXPECT_TRUE(ring.push(i));
    int value;
    EXPECT_TRUE(ring.pop(value));
    EXPECT_EQ(value, i);
    EXPECT_TRUE(ring.empty());
  }
}

TEST(SingleThreadedRingBuffer, SingleThreadFrontPtrTest) {
  int numItems = 100;
  SingleThreadedRingBuffer<int> ring(numItems + 1);
  for (int i = 0; i < numItems; i++) {
    EXPECT_TRUE(ring.push(i));
    const int front = *ring.front();
    EXPECT_EQ(front, i);
    int value;
    EXPECT_TRUE(ring.pop(value));
    EXPECT_EQ(value, i);
    EXPECT_TRUE(ring.empty());
  }
}

TEST(SingleThreadedRingBuffer, SingleThreadReadTest) {
  int numItems = 100;
  SingleThreadedRingBuffer<int> ring(numItems + 1);
  for (int i = 0; i < numItems; i++) {
    EXPECT_TRUE(ring.push(i));
    int* front = ring.front();
    EXPECT_EQ(static_cast<const int>(*front), i);
    int value;
    EXPECT_TRUE(ring.pop(value));
    EXPECT_EQ(value, i);
    EXPECT_TRUE(ring.empty());
  }
}

int main(int argc, char* argv[]) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
