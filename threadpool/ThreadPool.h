#pragma once

#include <atomic>
#include <condition_variable>
#include <deque>
#include <functional>
#include <mutex>
#include <thread>
#include <utility>
#include <vector>

// Implement a thread-safe queue using only a mutex and standard containers
class BasicThreadSafeQueue {
  std::deque<std::function<void()>> _queue;
  std::mutex _mutex;
  bool _done{false};

 public:
  bool pop(std::function<void()>& x) {
    // We try to acquire the lock without blocking. If we fail, we just return.
    // The caller needs to keep trying until the call succeeds.
    std::unique_lock<std::mutex> lock{_mutex, std::try_to_lock};
    if (!lock || _queue.empty()) {
      return false;
    }
    x = std::move(_queue.front());
    _queue.pop_front();
    return true;
  }

  template <typename F>
  bool push(F&& f) {
    // Same idea as pop. We try to acquire the lock without blocking. If we fail,
    // we just return. The caller needs to keep trying until the call succeeds.
    {
      std::unique_lock<std::mutex> lock{_mutex, std::try_to_lock};
      if (!lock) {
        return false;
      }
      _queue.emplace_back(std::forward<F>(f));
    }
    return true;
  }

  bool done() {
    std::unique_lock<std::mutex> lock{_mutex};
    _done = true;
    return true;
  }

  bool is_done() {
    std::unique_lock<std::mutex> lock{_mutex};
    return _done;
  }
};

class BasicThreadPool {
  const unsigned _nthreads{std::thread::hardware_concurrency()};
  std::vector<std::thread> _threads;
  BasicThreadSafeQueue _queue;

  void run() {
    while (true) {
      std::function<void()> task;
      if (_queue.is_done()) {
        break;
      }
      // Block until we're able to pop a task
      if (!_queue.pop(task)) {
        std::this_thread::yield();
        continue;
      }
      task();
    }
  }

 public:
  BasicThreadPool() {
    _threads.reserve(_nthreads);
    for (unsigned i = 0; i != _nthreads; ++i) {
      _threads.emplace_back([&]() { run(); });
    }
  }

  ~BasicThreadPool() {
    _queue.done();  // NOTE: Without this we hang in the dtor
    for (auto& t : _threads) {
      t.join();
    }
  }

  template <typename F>
  void submit(F&& f) {
    // Block until we're able to push a task
    while (!_queue.push(std::forward<F>(f))) {
      std::this_thread::yield();
    }
  }
};

// Implement a thread-safe queue using a mutex and a condition variable
class SimpleThreadSafeQueue {
  std::deque<std::function<void()>> _queue;
  std::mutex _mutex;
  std::condition_variable _ready;
  bool _done{false};

 public:
  bool pop(std::function<void()>& x) {
    std::unique_lock<std::mutex> lock{_mutex};
    // NOTE: Without a 'done' function we wait here forever
    while (_queue.empty() && !_done) {
      _ready.wait(lock);  // FIXME: lock contention
    }
    if (_queue.empty()) {
      return false;
    }
    x = std::move(_queue.front());
    _queue.pop_front();
    return true;
  }

  template <typename F>
  void push(F&& f) {
    {
      std::unique_lock<std::mutex> lock{_mutex};
      _queue.emplace_back(std::forward<F>(f));
    }
    _ready.notify_one();
  }

  void done() {
    {
      std::unique_lock<std::mutex> lock{_mutex};
      _done = true;
    }
    _ready.notify_all();
  }
};

// Implement a thread pool using the SimpleThreadSafeQueue
class SimpleThreadPool {
  const unsigned _nthreads{std::thread::hardware_concurrency()};
  std::vector<std::thread> _threads;
  SimpleThreadSafeQueue _queue;

  void run() {
    while (true) {
      std::function<void()> task;
      // NOTE: Without this we crash. We pass an empty function to the pop function, and since the queue is empty, this task never gets initialized/populated. We then try to call it, and crash.
      if (!_queue.pop(task)) {
        break;
      }
      task();
    }
  }

 public:
  SimpleThreadPool() {
    _threads.reserve(_nthreads);
    for (unsigned i = 0; i != _nthreads; ++i) {
      _threads.emplace_back([&]() { run(); });
    }
  }

  ~SimpleThreadPool() {
    _queue.done();  // NOTE: Without this we hang in the dtor
    for (auto& t : _threads) {
      t.join();
    }
  }

  template <typename F>
  void submit(F&& f) {
    _queue.push(std::forward<F>(f));
  }
};

// Implement a non-blocking thread-safe queue
class ThreadSafeQueue {
  std::deque<std::function<void()>> _queue;
  std::mutex _mutex;
  std::condition_variable _ready;
  bool _done{false};

 public:
  bool try_pop(std::function<void()>& x) {
    // We try to acquire the lock without blocking. If we fail, we just return.
    // The caller needs to try to pop from a different queue or wait.
    std::unique_lock<std::mutex> lock{_mutex, std::try_to_lock};
    if (!lock || _queue.empty()) {
      return false;
    }
    x = std::move(_queue.front());
    _queue.pop_front();
    return true;
  }

  template <typename F>
  bool try_push(F&& f) {
    // Same idea as try_pop.
    {
      std::unique_lock<std::mutex> lock{_mutex, std::try_to_lock};
      if (!lock) {
        return false;
      }
      _queue.emplace_back(std::forward<F>(f));
    }
    _ready.notify_one();
    return true;
  }

  void done() {
    {
      std::unique_lock<std::mutex> lock{_mutex};
      _done = true;
    }
    _ready.notify_all();
  }

  bool pop(std::function<void()>& x) {
    // Block until we're able to pop a task. Used when we must pop a task from this queue.
    std::unique_lock<std::mutex> lock{_mutex};
    while (_queue.empty() && !_done) {
      _ready.wait(lock);
    }
    if (_queue.empty()) {
      return false;
    }
    x = std::move(_queue.front());
    _queue.pop_front();
    return true;
  }

  template <typename F>
  void push(F&& f) {
    // Same idea as pop.
    {
      std::unique_lock<std::mutex> lock{_mutex};
      _queue.emplace_back(std::forward<F>(f));
    }
    _ready.notify_one();
  }
};

// Implement a work-stealing thread pool
class ThreadPool {
  const unsigned _nthreads{std::thread::hardware_concurrency()};
  std::vector<std::thread> _threads;
  std::vector<ThreadSafeQueue> _queues{_nthreads};
  std::atomic<unsigned> _index{0};
  const unsigned KMaxIterations = 32;

  void run(unsigned i) {
    while (true) {
      std::function<void()> f;
      // Try to pop from any queue that has tasks available.
      for (unsigned n = 0; n != _nthreads * KMaxIterations; ++n) {
        if (_queues[(i + n) % _nthreads].try_pop(f)) {
          break;
        }
      }
      // If we didn't find a task, try to pop from our own queue.
      if (!f && !_queues[i].pop(f)) {
        break;
      }
      f();
    }
  }

 public:
  ThreadPool() {
    _threads.reserve(_nthreads);
    for (unsigned n = 0; n != _nthreads; ++n) {
      _threads.emplace_back([&, n] { run(n); });
    }
  }

  ~ThreadPool() {
    for (auto& e : _queues) {
      e.done();
    }
    for (auto& e : _threads) {
      e.join();
    }
  }

  template <typename F>
  void submit(F&& f) {
    auto i = _index++;
    // Try to push to any queue that is not blocked.
    for (unsigned n = 0; n != _nthreads; ++n) {
      if (_queues[(i + n) % _nthreads].try_push(std::forward<F>(f))) {
        return;
      }
    }
    // If we couldn't push to any queue, push to our own queue.
    _queues[i % _nthreads].push(std::forward<F>(f));
  }
};
