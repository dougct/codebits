#include <algorithm>
#include <numeric>
#include <random>


template <typename I>
void print_range(I first, I last) {
    while (first != last) {
        std::cout << *first << " ";
        ++first;
    }
    std::cout << std::endl;
}

template <typename I>
void random_iota(I first, I last) {
    std::iota(first, last, 0);
    std::shuffle(first, last, std::mt19937{std::random_device{}()});
}

int rand_int(int lower, int upper) {
  std::random_device rand_dev;
  std::mt19937 generator(rand_dev());
  std::uniform_int_distribution<int> distrib(lower, upper);
  return distrib(generator);
}
