#pragma once

#include <atomic>
#include <condition_variable>
#include <deque>
#include <functional>
#include <mutex>
#include <thread>
#include <utility>
#include <vector>


class BasicThreadSafeQueue {
  std::deque<std::function<void()>> _queue;
  std::mutex _mutex;
  bool _done{false};

 public:
  bool pop(std::function<void()>& x) {
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
    _queue.done();  // XXX: Without this we hang in the dtor
    for (auto& t : _threads) {
      t.join();
    }
  }

  template <typename F>
  void submit(F&& f) {
    while (!_queue.push(std::forward<F>(f))) {
      std::this_thread::yield();
    }
  }
};

class SimpleThreadSafeQueue {
  std::deque<std::function<void()>> _queue;
  std::mutex _mutex;
  std::condition_variable _ready;
  bool _done{false};

 public:
  bool pop(std::function<void()>& x) {
    std::unique_lock<std::mutex> lock{_mutex};
    // XXX: Without a 'done' function we wait here forever
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

class SimpleThreadPool {
  const unsigned _nthreads{std::thread::hardware_concurrency()};
  std::vector<std::thread> _threads;
  SimpleThreadSafeQueue _queue;

  void run() {
    while (true) {
      std::function<void()> task;
      // Without this we crash. We pass an empty function to the pop function,
      // and since the queue is empty, this task never gets
      // initialized/populated. We then try to call it, and crash.
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
    _queue.done();  // XXX: Without this we hang in the dtor
    for (auto& t : _threads) {
      t.join();
    }
  }

  template <typename F>
  void submit(F&& f) {
    _queue.push(std::forward<F>(f));
  }
};

class ThreadSafeQueue {
  std::deque<std::function<void()>> _queue;
  std::mutex _mutex;
  std::condition_variable _ready;
  bool _done{false};

 public:
  bool try_pop(std::function<void()>& x) {
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
    {
      std::unique_lock<std::mutex> lock{_mutex};
      _queue.emplace_back(std::forward<F>(f));
    }
    _ready.notify_one();
  }
};

class ThreadPool {
  const unsigned _nthreads{std::thread::hardware_concurrency()};
  std::vector<std::thread> _threads;
  std::vector<ThreadSafeQueue> _queues{_nthreads};
  std::atomic<unsigned> _index{0};
  const unsigned KMaxIterations = 32;

  void run(unsigned i) {
    while (true) {
      std::function<void()> f;
      for (unsigned n = 0; n != _nthreads * KMaxIterations; ++n) {
        if (_queues[(i + n) % _nthreads].try_pop(f)) {
          break;
        }
      }
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
    for (unsigned n = 0; n != _nthreads; ++n) {
      if (_queues[(i + n) % _nthreads].try_push(std::forward<F>(f))) {
        return;
      }
    }
    _queues[i % _nthreads].push(std::forward<F>(f));
  }
};
