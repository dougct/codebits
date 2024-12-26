#include <forward_list>
#include <iostream>
#include <list>
#include <vector>

#include <benchmark/benchmark.h>

#include "rotate.h"
#include "utils.h"

template <typename Container>
Container prepare_data(size_t n) {
  std::vector<int> nums(n, 0);
  random_iota(nums.begin(), nums.end());
  return Container(nums.begin(), nums.end());
}

// Fwd list rotation benchmark
static void BM_RotateFwd_FwdList(benchmark::State& state) {
  const size_t n = state.range(0);
  const size_t k = n / 3;

  for (auto _ : state) {
    state.PauseTiming();
    auto data = prepare_data<std::forward_list<int>>(n);
    auto pivot = data.begin();
    std::advance(pivot, k);
    state.ResumeTiming();

    rotate_forward(data.begin(), pivot, data.end());
  }
  state.SetItemsProcessed(state.iterations() * n);
}
BENCHMARK(BM_RotateFwd_FwdList)->Range(1 << 16, 1 << 20)->RangeMultiplier(4);

// List rotation benchmarks
static void BM_RotateFwd_List(benchmark::State& state) {
  const size_t n = state.range(0);
  const size_t k = n / 3;

  for (auto _ : state) {
    state.PauseTiming();
    auto data = prepare_data<std::list<int>>(n);
    auto pivot = data.begin();
    std::advance(pivot, k);
    state.ResumeTiming();

    rotate_forward(data.begin(), pivot, data.end());
  }
  state.SetItemsProcessed(state.iterations() * n);
}
BENCHMARK(BM_RotateFwd_List)->Range(1 << 16, 1 << 20)->RangeMultiplier(4);

static void BM_RotateBidir_List(benchmark::State& state) {
  const size_t n = state.range(0);
  const size_t k = n / 3;

  for (auto _ : state) {
    state.PauseTiming();
    auto data = prepare_data<std::list<int>>(n);
    auto pivot = data.begin();
    std::advance(pivot, k);
    state.ResumeTiming();

    rotate_bidirectional(data.begin(), pivot, data.end());
  }
  state.SetItemsProcessed(state.iterations() * n);
}
BENCHMARK(BM_RotateBidir_List)->Range(1 << 16, 1 << 20)->RangeMultiplier(4);

// Vector rotation benchmarks
static void BM_RotateFwd_Vector(benchmark::State& state) {
  const size_t n = state.range(0);
  const size_t k = n / 3;

  for (auto _ : state) {
    state.PauseTiming();
    auto data = prepare_data<std::vector<int>>(n);
    auto pivot = data.begin();
    std::advance(pivot, k);
    state.ResumeTiming();

    rotate_forward(data.begin(), pivot, data.end());
  }
  state.SetItemsProcessed(state.iterations() * n);
}
BENCHMARK(BM_RotateFwd_Vector)->Range(1 << 16, 1 << 20)->RangeMultiplier(4);

static void BM_RotateBidir_Vector(benchmark::State& state) {
  const size_t n = state.range(0);
  const size_t k = n / 3;

  for (auto _ : state) {
    state.PauseTiming();
    auto data = prepare_data<std::vector<int>>(n);
    auto pivot = data.begin();
    std::advance(pivot, k);
    state.ResumeTiming();

    rotate_bidirectional(data.begin(), pivot, data.end());
  }
  state.SetItemsProcessed(state.iterations() * n);
}
BENCHMARK(BM_RotateBidir_Vector)->Range(1 << 16, 1 << 20)->RangeMultiplier(4);

static void BM_RotateGCD_Vector(benchmark::State& state) {
  const size_t n = state.range(0);
  const size_t k = n / 3;

  for (auto _ : state) {
    state.PauseTiming();
    auto data = prepare_data<std::vector<int>>(n);
    auto pivot = data.begin();
    std::advance(pivot, k);
    state.ResumeTiming();

    rotate_gcd(data.begin(), pivot, data.end());
  }
  state.SetItemsProcessed(state.iterations() * n);
}
BENCHMARK(BM_RotateGCD_Vector)->Range(1 << 16, 1 << 20)->RangeMultiplier(4);

// Benchmark comparing against std::rotate
static void BM_Rotate_STD(benchmark::State& state) {
  const size_t n = state.range(0);
  const size_t k = n / 3;

  for (auto _ : state) {
    state.PauseTiming();
    auto data = prepare_data<std::vector<int>>(n);
    auto pivot = data.begin();
    std::advance(pivot, k);
    state.ResumeTiming();

    std::rotate(data.begin(), pivot, data.end());
  }
  state.SetItemsProcessed(state.iterations() * n);
}
BENCHMARK(BM_Rotate_STD)->Range(1 << 16, 1 << 20)->RangeMultiplier(4);

BENCHMARK_MAIN();
