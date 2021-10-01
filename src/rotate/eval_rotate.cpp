#include <algorithm>
#include <cassert>
#include <chrono>
#include <forward_list>
#include <iomanip>
#include <iostream>
#include <list>
#include <numeric>
#include <random>
#include <type_traits>
#include <vector>
#include <tuple>

#include "rotate.h"
#include "timing.h"
#include "utils.h"

template <typename I>
uint64_t eval_rotate_fwd(I first, I middle, I last) {
  auto before = milliseconds();
  rotate_forward(first, middle, last);
  auto after = milliseconds();
  return after - before;
}

template <typename I>
uint64_t eval_rotate_bid(I first, I middle, I last) {
  auto before = milliseconds();
  rotate_bidirectional(first, middle, last);
  auto after = milliseconds();
  return after - before;
}

template <typename I>
uint64_t eval_rotate_gcd(I first, I middle, I last) {
  auto before = milliseconds();
  rotate_gcd(first, middle, last);
  auto after = milliseconds();
  return after - before;
}

void check_correcness(size_t n, size_t k) {
  std::vector<int> v_std(n, 0);
  random_iota(v_std.begin(), v_std.end());

  std::vector<int> v_fwd(v_std.begin(), v_std.end());
  std::vector<int> v_bid(v_std.begin(), v_std.end());
  std::vector<int> v_rand(v_std.begin(), v_std.end());
  std::vector<int> v_rs(v_std.begin(), v_std.end());

  auto s = std::rotate(v_std.begin(), v_std.begin() + k, v_std.end());

  auto f = rotate_forward(v_fwd.begin(), v_fwd.begin() + k, v_fwd.end());
  assert(v_fwd == v_std);
  assert(*f == *s);

  auto b = rotate_bidirectional(v_bid.begin(), v_bid.begin() + k, v_bid.end());
  assert(v_bid == v_std);
  assert(*b == *s);

  auto r = rotate_gcd_stepanov(v_rand.begin(), v_rand.begin() + k, v_rand.end());
  assert(v_rand == v_std);
  assert(*r == *s);

  auto rs = rotate_gcd(v_rs.begin(), v_rs.begin() + k, v_rs.end());
  assert(v_rs == v_std);
  assert(*rs == *s);
}

int main() {
  std::ios_base::sync_with_stdio(false);

  double total_fwd_list_fwd = 0.0, total_lst_fwd = 0.0, total_vec_fwd = 0.0;
  double total_lst_bid = 0.0, total_vec_bid = 0.0;
  double total_vec_gcd = 0.0;
  auto n = 1000000, times = 100;
  for (int i = 0; i < times; ++i) {
    // How many elements we're going to rotate
    auto k = rand_int(0, n - 1);

    // Make sure our implementations yield the same results as std::rotate
    check_correcness(n, k);

    // Create a vector whose copies will be rotated by several algorithms
    std::vector<int> nums(n, 0);
    random_iota(nums.begin(), nums.end());

    // Evaluate rotate_forward with different containers

    std::forward_list<int> fwd_list(nums.begin(), nums.end());
    auto fwd_kth = fwd_list.begin();
    std::advance(fwd_kth, k);
    auto fwd_time = eval_rotate_fwd(fwd_list.begin(), fwd_kth, fwd_list.end());
    total_fwd_list_fwd += fwd_time;

    std::list<int> lst_fwd(nums.begin(), nums.end());
    auto lst_fwd_kth = lst_fwd.begin();
    std::advance(lst_fwd_kth, k);
    auto lst_fwd_time = eval_rotate_fwd(lst_fwd.begin(), lst_fwd_kth, lst_fwd.end());
    total_lst_fwd += lst_fwd_time;

    std::vector<int> vec_fwd(nums.begin(), nums.end());
    auto vec_fwd_kth = vec_fwd.begin();
    std::advance(vec_fwd_kth, k);
    auto vec_fwd_time = eval_rotate_fwd(vec_fwd.begin(), vec_fwd_kth, vec_fwd.end());
    total_vec_fwd += vec_fwd_time;

    // Evaluate rotate bidirectional with list and vector

    std::list<int> lst_bid(nums.begin(), nums.end());
    auto lst_bid_kth = lst_bid.begin();
    std::advance(lst_bid_kth, k);
    auto lst_bid_time = eval_rotate_bid(lst_bid.begin(), lst_bid_kth, lst_bid.end());
    total_lst_bid += lst_bid_time;

    std::vector<int> vec_bid(nums.begin(), nums.end());
    auto vec_bid_kth = vec_bid.begin();
    std::advance(vec_bid_kth, k);
    auto vec_bid_time = eval_rotate_bid(vec_bid.begin(), vec_bid_kth, vec_bid.end());
    total_vec_bid += vec_bid_time;

    // Evaluate rotate gdc with vector, since only vector supports random access iterators
    
    std::vector<int> vec_gcd(nums.begin(), nums.end());
    auto vec_gcd_kth = vec_gcd.begin();
    std::advance(vec_gcd_kth, k);
    auto vec_gcd_time = eval_rotate_gcd(vec_gcd.begin(), vec_gcd_kth, vec_gcd.end());
    total_vec_gcd += vec_gcd_time;
  }

  std::cout << std::setprecision(4) << std::fixed;

  std::cout << "Algorithm: rotate_forward" << std::endl;
  std::cout << "    Data structure: fwd_list: " << total_fwd_list_fwd / times << std::endl;
  std::cout << "    Data structure: list: " << total_lst_fwd / times << std::endl;
  std::cout << "    Data structure: vector: " << total_vec_fwd / times << std::endl;

  std::cout << "Algorithm: rotate_bidirectional" << std::endl;
  std::cout << "    Data structure: list: " << total_lst_bid / times << std::endl;
  std::cout << "    Data structure: vector: " << total_vec_bid / times << std::endl;

  std::cout << "Algorithm: rotate_gcd" << std::endl;
  std::cout << "    Data structure: vector: " << total_vec_gcd / times << std::endl;

  return 0;
}