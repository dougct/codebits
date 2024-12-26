#include <algorithm>
#include <vector>

#include <gtest/gtest.h>

#include "Rotate.h"
#include "Utils.h"

class RotateTest : public ::testing::Test {
 protected:
  void SetUp() override {}

  std::vector<int> prepare_test_data(size_t n) {
    std::vector<int> vec(n, 0);
    random_iota(vec.begin(), vec.end());
    return vec;
  }
};

TEST_F(RotateTest, CorrectnessTest) {
  auto n = 1000000, times = 300;
  for (int i = 0; i < times; ++i) {
    std::vector<int> v_std = prepare_test_data(n);

    std::vector<int> v_fwd(v_std.begin(), v_std.end());
    std::vector<int> v_bid(v_std.begin(), v_std.end());
    std::vector<int> v_rand(v_std.begin(), v_std.end());
    std::vector<int> v_rs(v_std.begin(), v_std.end());

    // How many elements we're going to rotate
    auto k = rand_int(0, n - 1);

    auto s = std::rotate(v_std.begin(), v_std.begin() + k, v_std.end());

    auto f = rotate_forward(v_fwd.begin(), v_fwd.begin() + k, v_fwd.end());
    EXPECT_EQ(v_fwd, v_std);
    EXPECT_EQ(*f, *s);

    auto b =
        rotate_bidirectional(v_bid.begin(), v_bid.begin() + k, v_bid.end());
    EXPECT_EQ(v_bid, v_std);
    EXPECT_EQ(*b, *s);

    auto r =
        rotate_gcd_stepanov(v_rand.begin(), v_rand.begin() + k, v_rand.end());
    EXPECT_EQ(v_rand, v_std);
    EXPECT_EQ(*r, *s);

    auto rs = rotate_gcd(v_rs.begin(), v_rs.begin() + k, v_rs.end());
    EXPECT_EQ(v_rs, v_std);
    EXPECT_EQ(*rs, *s);
  }
}

int main(int argc, char** argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
