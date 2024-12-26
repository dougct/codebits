#include <atomic>
#include <thread>

#include <benchmark/benchmark.h>

#include "RingBuffer.h"
#include "SingleThreadedRingBuffer.h"

template <typename BufferType>
static void BM_RingBuffer(benchmark::State& state) {
  for (auto _ : state) {
    state.PauseTiming();
    const size_t iter = state.range(0);
    BufferType ring(iter / 1000 + 1);
    std::atomic<bool> flag(false);
    long sum = 0;

    state.ResumeTiming();

    std::thread producer([&] {
      while (!flag) {
        std::this_thread::yield();
      }

      size_t i = 0;
      while (i < iter) {
        if (ring.push(i)) {
          i++;
        }
      }
    });

    flag = true;
    for (size_t i = 0; i < iter; ++i) {
      while (!ring.front()) {
        std::this_thread::yield();
      }
      size_t value;
      ring.pop(value);
      sum += value;
    }

    producer.join();
    benchmark::DoNotOptimize(sum);
    benchmark::ClobberMemory();
  }
}

BENCHMARK_TEMPLATE(BM_RingBuffer, SingleThreadedRingBuffer<size_t>)
    ->Range(1 << 16, 1 << 24)
    ->RangeMultiplier(2)
    ->UseRealTime();

BENCHMARK_TEMPLATE(BM_RingBuffer, RingBuffer<size_t>)
    ->Range(1 << 16, 1 << 24)
    ->RangeMultiplier(2)
    ->UseRealTime();

BENCHMARK_MAIN();
