# Benchmark results

```
----------------------------------------------------------------------------------------------------------
Benchmark                                                Time             CPU   Iterations UserCounters...
----------------------------------------------------------------------------------------------------------
BM_ExactCounterSingleThreaded                         9.00 ns         9.00 ns     77513371 items_per_second=111.158M/s
BM_ExactCounterMultiThreaded/8/real_time              2350 ns         2327 ns       291285 items_per_second=3.40497M/s
BM_ExactCounterMultiThreaded/16/real_time             2457 ns         2429 ns       286113 items_per_second=6.5113M/s
BM_ApproxCounterSingleThreaded                        17.1 ns         17.1 ns     40482786 items_per_second=58.4054M/s
BM_ApproxCounterMultiThreaded/8/1024/real_time        2346 ns         2323 ns       297551 items_per_second=3.41029M/s
BM_ApproxCounterMultiThreaded/16/1024/real_time       2509 ns         2469 ns       284297 items_per_second=6.37694M/s
```

# Notes

Version of `update` with a bug:

```cpp
// All variables are atomic
int64_t update(int64_t amount) {
    // Round-robin updating local counters
    const uint32_t idx = num_updates_++ % num_threads_;
    local_counters_[idx] += amount;
    if (num_updates_ >= threshold_) {
        global_counter_ += local_counters_[idx];
        local_counters_[idx] = 0;
        num_updates_ = 0;
    }
    return global_counter_;
}
```

This version was overcounting in the ApproxCounter because even though the increments are atomic, there could be some thread interleavings which lead to double counting.
