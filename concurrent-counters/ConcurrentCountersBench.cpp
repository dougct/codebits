#include <atomic>
#include <cstdint>
#include <thread>
#include <vector>

#include <benchmark/benchmark.h>

#include "ApproxCounter.h"
#include "ExactCounter.h"

// ExactCounter benchmarks
static void BM_ExactCounterSingleThreaded(benchmark::State& state) {
  ExactCounter counter;
  for (auto _ : state) {
    counter.update(1);
  }
  state.SetItemsProcessed(state.iterations());
}

static void BM_ExactCounterMultiThreaded(benchmark::State& state) {
  ExactCounter counter;
  const int num_threads = state.range(0);

  for (auto _ : state) {
    state.PauseTiming();
    std::vector<std::thread> threads;
    std::atomic<bool> start{false};
    std::atomic<int> threads_ready{0};

    // Create threads
    for (int i = 0; i < num_threads; ++i) {
      threads.emplace_back([&counter, &start, &threads_ready] {
        threads_ready++;
        while (!start) {
          std::this_thread::yield();
        }
        counter.update(1);
      });
    }

    // Wait for all threads to be ready
    while (threads_ready < num_threads) {
      std::this_thread::yield();
    }

    state.ResumeTiming();
    start = true;

    state.PauseTiming();
    for (auto& thread : threads) {
      thread.join();
    }
    state.ResumeTiming();
  }

  state.SetItemsProcessed(state.iterations() * num_threads);
}

// ApproxCounter benchmarks
static void BM_ApproxCounterSingleThreaded(benchmark::State& state) {
  ApproxCounter counter(1, 1);
  for (auto _ : state) {
    counter.update(1);
  }
  state.SetItemsProcessed(state.iterations());
}

static void BM_ApproxCounterMultiThreaded(benchmark::State& state) {
  const int num_threads = state.range(0);
  const int threshold = state.range(1);
  ApproxCounter counter(threshold, num_threads);

  for (auto _ : state) {
    state.PauseTiming();
    std::vector<std::thread> threads;
    std::atomic<bool> start{false};
    std::atomic<int> threads_ready{0};

    // Create threads
    for (int i = 0; i < num_threads; ++i) {
      threads.emplace_back([&counter, &start, &threads_ready] {
        threads_ready++;
        while (!start) {
          std::this_thread::yield();
        }
        counter.update(1);
      });
    }

    // Wait for all threads to be ready
    while (threads_ready < num_threads) {
      std::this_thread::yield();
    }

    state.ResumeTiming();
    start = true;

    state.PauseTiming();
    for (auto& thread : threads) {
      thread.join();
    }
    state.ResumeTiming();
  }

  state.SetItemsProcessed(state.iterations() * num_threads);
}

// Register ExactCounter benchmarks
BENCHMARK(BM_ExactCounterSingleThreaded);
BENCHMARK(BM_ExactCounterMultiThreaded)
    ->RangeMultiplier(2)
    ->Range(1, 2 * std::thread::hardware_concurrency())
    ->UseRealTime();

// Register ApproxCounter benchmarks
BENCHMARK(BM_ApproxCounterSingleThreaded);
BENCHMARK(BM_ApproxCounterMultiThreaded)
    ->RangeMultiplier(2)
    ->Ranges({{8, 2 * std::thread::hardware_concurrency()}, {512, 1024}})
    ->UseRealTime();

BENCHMARK_MAIN();
