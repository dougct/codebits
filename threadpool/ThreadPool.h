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
  std::deque<std::function<void()>> queue_;
  std::mutex mutex_;
  bool done_{false};

 public:
  bool pop(std::function<void()>& x) {
    // We try to acquire the lock without blocking. If we fail, we just return.
    // The caller needs to keep trying until the call succeeds.
    std::unique_lock<std::mutex> lock{mutex_, std::try_to_lock};
    if (!lock || queue_.empty()) {
      return false;
    }
    x = std::move(queue_.front());
    queue_.pop_front();
    return true;
  }

  template <typename F>
  bool push(F&& f) {
    // Same idea as pop. We try to acquire the lock without blocking. If we
    // fail, we just return. The caller needs to keep trying until the call
    // succeeds.
    {
      std::unique_lock<std::mutex> lock{mutex_, std::try_to_lock};
      if (!lock) {
        return false;
      }
      queue_.emplace_back(std::forward<F>(f));
    }
    return true;
  }

  bool done() {
    std::unique_lock<std::mutex> lock{mutex_};
    done_ = true;
    return true;
  }

  bool is_done() {
    std::unique_lock<std::mutex> lock{mutex_};
    return done_;
  }
};

class BasicThreadPool {
  const unsigned nthreads_{std::thread::hardware_concurrency()};
  std::vector<std::thread> threads_;
  BasicThreadSafeQueue queue_;

  void run() {
    while (true) {
      std::function<void()> task;
      if (queue_.is_done()) {
        break;
      }
      // Block until we're able to pop a task
      if (!queue_.pop(task)) {
        std::this_thread::yield();
        continue;
      }
      task();
    }
  }

 public:
  BasicThreadPool() {
    threads_.reserve(nthreads_);
    for (unsigned i = 0; i != nthreads_; ++i) {
      threads_.emplace_back([&]() { run(); });
    }
  }

  ~BasicThreadPool() {
    queue_.done();  // NOTE: Without this we hang in the dtor
    for (auto& t : threads_) {
      t.join();
    }
  }

  template <typename F>
  void submit(F&& f) {
    // Block until we're able to push a task
    while (!queue_.push(std::forward<F>(f))) {
      std::this_thread::yield();
    }
  }
};

// Implement a thread-safe queue using a mutex and a condition variable
class SimpleThreadSafeQueue {
  std::deque<std::function<void()>> queue_;
  std::mutex mutex_;
  std::condition_variable ready_;
  bool done_{false};

 public:
  bool pop(std::function<void()>& x) {
    std::unique_lock<std::mutex> lock{mutex_};
    // NOTE: Without a 'done' function we wait here forever
    while (queue_.empty() && !done_) {
      ready_.wait(lock);  // FIXME: lock contention
    }
    if (queue_.empty()) {
      return false;
    }
    x = std::move(queue_.front());
    queue_.pop_front();
    return true;
  }

  template <typename F>
  void push(F&& f) {
    {
      std::unique_lock<std::mutex> lock{mutex_};
      queue_.emplace_back(std::forward<F>(f));
    }
    ready_.notify_one();
  }

  void done() {
    {
      std::unique_lock<std::mutex> lock{mutex_};
      done_ = true;
    }
    ready_.notify_all();
  }
};

// Implement a thread pool using the SimpleThreadSafeQueue
class SimpleThreadPool {
  const unsigned nthreads_{std::thread::hardware_concurrency()};
  std::vector<std::thread> threads_;
  SimpleThreadSafeQueue queue_;

  void run() {
    while (true) {
      std::function<void()> task;
      // NOTE: Without this we crash. We pass an empty function to the pop
      // function, and since the queue is empty, this task never gets
      // initialized/populated. We then try to call it, and crash.
      if (!queue_.pop(task)) {
        break;
      }
      task();
    }
  }

 public:
  SimpleThreadPool() {
    threads_.reserve(nthreads_);
    for (unsigned i = 0; i != nthreads_; ++i) {
      threads_.emplace_back([&]() { run(); });
    }
  }

  ~SimpleThreadPool() {
    queue_.done();  // NOTE: Without this we hang in the dtor
    for (auto& t : threads_) {
      t.join();
    }
  }

  template <typename F>
  void submit(F&& f) {
    queue_.push(std::forward<F>(f));
  }
};

// Implement a non-blocking thread-safe queue
class ThreadSafeQueue {
  std::deque<std::function<void()>> queue_;
  std::mutex mutex_;
  std::condition_variable ready_;
  bool done_{false};

 public:
  bool try_pop(std::function<void()>& x) {
    // We try to acquire the lock without blocking. If we fail, we just return.
    // The caller needs to try to pop from a different queue or wait.
    std::unique_lock<std::mutex> lock{mutex_, std::try_to_lock};
    if (!lock || queue_.empty()) {
      return false;
    }
    x = std::move(queue_.front());
    queue_.pop_front();
    return true;
  }

  template <typename F>
  bool try_push(F&& f) {
    // Same idea as try_pop.
    {
      std::unique_lock<std::mutex> lock{mutex_, std::try_to_lock};
      if (!lock) {
        return false;
      }
      queue_.emplace_back(std::forward<F>(f));
    }
    ready_.notify_one();
    return true;
  }

  void done() {
    {
      std::unique_lock<std::mutex> lock{mutex_};
      done_ = true;
    }
    ready_.notify_all();
  }

  bool pop(std::function<void()>& x) {
    // Block until we're able to pop a task. Used when we must pop a task from
    // this queue.
    std::unique_lock<std::mutex> lock{mutex_};
    while (queue_.empty() && !done_) {
      ready_.wait(lock);
    }
    if (queue_.empty()) {
      return false;
    }
    x = std::move(queue_.front());
    queue_.pop_front();
    return true;
  }

  template <typename F>
  void push(F&& f) {
    // Same idea as pop.
    {
      std::unique_lock<std::mutex> lock{mutex_};
      queue_.emplace_back(std::forward<F>(f));
    }
    ready_.notify_one();
  }
};

// Implement a work-stealing thread pool
class ThreadPool {
  const unsigned nthreads_{std::thread::hardware_concurrency()};
  std::vector<std::thread> threads_;
  std::vector<ThreadSafeQueue> queues_{nthreads_};
  std::atomic<unsigned> index_{0};
  const unsigned KMaxIterations = 32;

  void run(unsigned i) {
    while (true) {
      std::function<void()> f;
      // Try to pop from any queue that has tasks available.
      for (unsigned n = 0; n != nthreads_ * KMaxIterations; ++n) {
        if (queues_[(i + n) % nthreads_].try_pop(f)) {
          break;
        }
      }
      // If we didn't find a task, try to pop from our own queue.
      if (!f && !queues_[i].pop(f)) {
        break;
      }
      f();
    }
  }

 public:
  ThreadPool() {
    threads_.reserve(nthreads_);
    for (unsigned n = 0; n != nthreads_; ++n) {
      threads_.emplace_back([&, n] { run(n); });
    }
  }

  ~ThreadPool() {
    for (auto& e : queues_) {
      e.done();
    }
    for (auto& e : threads_) {
      e.join();
    }
  }

  template <typename F>
  void submit(F&& f) {
    auto i = index_++;
    // Try to push to any queue that is not blocked.
    for (unsigned n = 0; n != nthreads_; ++n) {
      if (queues_[(i + n) % nthreads_].try_push(std::forward<F>(f))) {
        return;
      }
    }
    // If we couldn't push to any queue, push to our own queue.
    queues_[i % nthreads_].push(std::forward<F>(f));
  }
};
