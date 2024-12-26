#include <atomic>
#include <chrono>
#include <numeric>
#include <random>
#include <thread>
#include <vector>

#include <benchmark/benchmark.h>

#include "ThreadPool.h"

// Benchmark task throughput
static void BM_TaskThroughput(benchmark::State& state) {
  ThreadPool pool;
  std::atomic<size_t> counter{0};

  for (auto _ : state) {
    counter = 0;
    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < state.range(0); ++i) {
      pool.submit(
          [&counter]() { counter.fetch_add(1, std::memory_order_relaxed); });
    }

    // Wait for all tasks to complete
    while (counter < static_cast<size_t>(state.range(0))) {
      std::this_thread::yield();
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto elapsed =
        std::chrono::duration_cast<std::chrono::microseconds>(end - start);

    state.SetIterationTime(elapsed.count() / 1e6);
    state.SetItemsProcessed(state.range(0));
  }
}
BENCHMARK(BM_TaskThroughput)->Range(1 << 10, 1 << 20)->UseRealTime();

BENCHMARK_MAIN();
