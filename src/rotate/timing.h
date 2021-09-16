#include <chrono>
#include <numeric>


uint64_t nanosecs() {
    auto now = std::chrono::steady_clock::now().time_since_epoch();
    return std::chrono::duration_cast<::std::chrono::nanoseconds>(now).count();
}

uint64_t milliseconds() {
    auto now = std::chrono::steady_clock::now().time_since_epoch();
    return std::chrono::duration_cast<::std::chrono::milliseconds>(now).count();
}

double secs_elapsed(uint64_t before, uint64_t after) {
    double secs = (after - before) / 1000000000.;
    return secs;
}

template<typename F, typename... Args>
uint64_t timed_execution(F func, Args&&... args) {
    auto before = nanosecs();
    func(std::forward<Args>(args)...);
    auto after = nanosecs();
    return after - before;
}

auto timed_execution_lambda = [](auto&& func, auto&&... params) {
    auto before = nanosecs();
    std::forward<decltype(func)>(func)(std::forward<decltype(params)>(params)...);
    auto after = nanosecs();
    return after - before;
};


// How to use the functions for timed execution

inline int foo(int k) {
    return k + 1;
}

uint64_t timed_rotate(int k) {
    return timed_execution(foo, k);
}
