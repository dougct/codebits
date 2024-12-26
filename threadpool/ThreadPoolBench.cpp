#include <atomic>
#include <chrono>
#include <future>
#include <numeric>
#include <random>
#include <thread>
#include <vector>

#include <benchmark/benchmark.h>

#include "ThreadPool.h"

template<typename PoolType>
static void BM_TaskThroughput(benchmark::State& state) {
    PoolType pool;
    std::atomic<size_t> counter{0};

    for (auto _ : state) {
        counter = 0;
        auto start = std::chrono::high_resolution_clock::now();

        for (int i = 0; i < state.range(0); ++i) {
            pool.submit([&counter]() {
                counter.fetch_add(1, std::memory_order_relaxed);
            });
        }

        // Wait for all tasks to complete
        while (counter < static_cast<size_t>(state.range(0))) {
            std::this_thread::yield();
        }

        auto end = std::chrono::high_resolution_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

        state.SetIterationTime(elapsed.count() / 1e3);
        state.SetItemsProcessed(state.range(0));
    }
}

// Register benchmarks for each thread pool type
BENCHMARK_TEMPLATE(BM_TaskThroughput, SimpleThreadPool)
    ->Range(1<<10, 1<<20)
    ->UseRealTime();

BENCHMARK_TEMPLATE(BM_TaskThroughput, BasicThreadPool)
    ->Range(1<<10, 1<<20)
    ->UseRealTime();

BENCHMARK_TEMPLATE(BM_TaskThroughput, ThreadPool)
    ->Range(1<<10, 1<<20)
    ->UseRealTime();

BENCHMARK_MAIN();
