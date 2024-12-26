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
