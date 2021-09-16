#include <algorithm>
#include <cassert>
#include <forward_list>
#include <iomanip>
#include <iostream>
#include <list>
#include <numeric>
#include <random>
#include <type_traits>
#include <vector>

#include "rotate.h"
#include "timing.h"
#include "utils.h"


std::pair<uint64_t, uint64_t> eval_rotate_forward_simple(size_t n, size_t k) {
    std::vector<int> nums(n, 0);
    random_iota(nums.begin(), nums.end());
    std::forward_list<int> nums_list(nums.begin(), nums.end());

    auto vec_before = milliseconds();
    rotate_forward_void(nums.begin(), nums.begin() + k, nums.end());
    auto vec_after = milliseconds();

    // Advance k positions in the list
    auto m = nums_list.begin();
    auto d = k;
    while (--d) {
        ++m;
    }

    auto list_before = milliseconds();
    rotate_forward_void(nums_list.begin(), m, nums_list.end());
    auto list_after = milliseconds();

    return {vec_after - vec_before, list_after - list_before};
}


std::pair<uint64_t, uint64_t> eval_rotate_forward(size_t n, size_t k) {
    std::vector<int> nums(n, 0);
    random_iota(nums.begin(), nums.end());
    std::forward_list<int> nums_list(nums.begin(), nums.end());

    auto vec_before = milliseconds();
    rotate_forward(nums.begin(), nums.begin() + k, nums.end());
    auto vec_after = milliseconds();

    // Advance k positions in the list
    auto m = nums_list.begin();
    auto d = k;
    while (--d) {
        ++m;
    }

    auto list_before = milliseconds();
    rotate_forward(nums_list.begin(), m, nums_list.end());
    auto list_after = milliseconds();

    return {vec_after - vec_before, list_after - list_before};
}


std::pair<uint64_t, uint64_t> eval_rotate_bidirectional(size_t n, size_t k) {
    std::vector<int> nums(n, 0);
    random_iota(nums.begin(), nums.end());
    std::list<int> nums_list(nums.begin(), nums.end());

    auto vec_before = milliseconds();
    rotate_bidirectional(nums.begin(), nums.begin() + k, nums.end());
    auto vec_after = milliseconds();

    // Advance k positions in the list
    auto m = nums_list.begin();
    auto d = k;
    while (d--) {
        ++m;
    }

    auto list_before = milliseconds();
    rotate_bidirectional(nums_list.begin(), m, nums_list.end());
    auto list_after = milliseconds();

    return {vec_after - vec_before, list_after - list_before};
}


void check_correcness(size_t n, size_t k) {
    std::vector<int> v_std(n, 0);
    random_iota(v_std.begin(), v_std.end());
    
    std::vector<int> v_fwd(v_std.begin(), v_std.end());
    std::vector<int> v_bid(v_std.begin(), v_std.end());
    std::vector<int> v_rand(v_std.begin(), v_std.end());
    
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
}


int main() {
    std::ios_base::sync_with_stdio(false);
    
    double total_fwd_vec = 0.0, total_fwd_list = 0.0; 
    double total_fwd_simple_vec = 0.0, total_fwd_simple_list = 0.0;
    double total_bid_vec = 0.0, total_bid_list = 0.0;

    auto n = 1000000, times = 100;
    for (int i = 0; i < times; ++i) {
        // How many elements we're going to rotate
        auto k = rand_int(0, n - 1);

        // Make sure our implementations yield the same results as std::rotate
        check_correcness(n, k);

        // Evaluate rotate implementations

        auto fs = eval_rotate_forward_simple(n, k);
        total_fwd_simple_vec += fs.first;
        total_fwd_simple_list += fs.second;

        auto f = eval_rotate_forward(n, k);
        total_fwd_vec += f.first;
        total_fwd_list += f.second;

        auto b = eval_rotate_bidirectional(n, k);
        total_bid_vec += b.first;
        total_bid_list += b.second;
    }

    std::cout << std::setprecision(4) << std::fixed;

    std::cout << "Algorithm: rotate_forward_simple" << std::endl;
    std::cout << "    Data structure: vector: " << total_fwd_simple_vec / times << std::endl;
    std::cout << "    Data structure: list: " << total_fwd_simple_list / times << std::endl;
    
    std::cout << "Algorithm: rotate_forward" << std::endl;
    std::cout << "    Data structure: vector: " << total_fwd_vec / times << std::endl;
    std::cout << "    Data structure: list: " << total_fwd_list / times << std::endl;

    std::cout << "Algorithm: rotate_bidirectional " << std::endl;
    std::cout << "    Data structure: vector: " << total_bid_vec / times << std::endl;
    std::cout << "    Data structure: list: " << total_bid_list / times << std::endl;

    return 0;
}