#include <benchmark/benchmark.h>
#include <thread>
#include <future>

size_t counter{0};

static void BM_EmptyThread(benchmark::State& state) {
    for (auto _ : state) {
        auto mythread = std::thread([] { });
        mythread.join();
    }
}
BENCHMARK(BM_EmptyThread);

static void BM_ThreadWithCounter(benchmark::State& state) {
    for (auto _ : state) {
        auto mythread = std::thread([] { counter++; });
        mythread.join();
    }
}
BENCHMARK(BM_ThreadWithCounter);

static void BM_AsyncWithCounter(benchmark::State& state) {
    for (auto _ : state) {
        auto f = std::async(std::launch::async, []{counter++;});
        f.get();
    }
}
BENCHMARK(BM_AsyncWithCounter);

BENCHMARK_MAIN();
