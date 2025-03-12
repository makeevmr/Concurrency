#include "../../source/mutex.hpp"
#include "../../source/condvar.hpp"

#include <course/test/twist.hpp>

#include <twist/test/body/either.hpp>
#include <twist/test/body/wg.hpp>

#include <twist/assist/assert.hpp>
#include <twist/assist/random.hpp>
#include <twist/assist/shared.hpp>

#include <atomic>
#include <deque>
#include <optional>
#include <mutex>

inline void NotifyAtLeastOne(CondVar& cv) {
  if (twist::test::body::Either()) {
    cv.NotifyOne();
  } else {
    cv.NotifyAll();
  }
}

class Event {
 public:
  void Wait() {
    std::unique_lock lock{mutex_};
    while (!ready_.Read()) {
      fire_.Wait(lock);
    }
  }

  void Fire() {
    std::lock_guard guard{mutex_};
    ready_.Write(true);
    fire_.NotifyAll();
  }

 private:
  Mutex mutex_;
  twist::assist::Shared<bool> ready_{false};
  CondVar fire_;
};

class Semaphore {
 public:
  explicit Semaphore(size_t init = 0)
      : count_(init) {
  }

  void Acquire() {
    std::unique_lock lock{mutex_};
    while (count_.Read() == 0) {
      release_.Wait(lock);
    }
    count_.Write(count_.Read() - 1);
  }

  void Release() {
    std::unique_lock lock{mutex_};
    count_.Write(count_.Read() + 1);

    if (twist::test::body::Either()) {
      lock.unlock();
    }

    release_.NotifyOne();
  }

 private:
  twist::assist::Shared<size_t> count_;
  Mutex mutex_;
  CondVar release_;
};

class Queue {
 public:
  explicit Queue(size_t capacity)
      : capacity_(capacity) {
  }

  void Push(int datum) {
    std::unique_lock lock{mutex_};
    while (items_->size() == capacity_) {
      not_full_.Wait(lock);
    }
    items_->push_back(datum);

    if (twist::test::body::Either()) {
      lock.unlock();
    }

    NotifyAtLeastOne(not_empty_);
  }

  std::optional<int> Pop() {
    std::unique_lock lock{mutex_};
    while (items_->empty() && !closed_) {
      not_empty_.Wait(lock);
    }
    if (!items_->empty()) {
      int datum = items_->front();
      items_->pop_front();
      not_full_.NotifyOne();
      return datum;
    } else {
      return std::nullopt;
    }
  }

  void Close() {
    std::lock_guard guard{mutex_};
    closed_ = true;
    not_full_.NotifyAll();
    not_empty_.NotifyAll();
  }

 private:
  const size_t capacity_;
  Mutex mutex_;
  twist::assist::Shared<std::deque<int>> items_;
  bool closed_ = false;
  CondVar not_full_;
  CondVar not_empty_;
};

class Barrier {
 public:
  explicit Barrier(size_t threads)
      : threads_(threads),
        left_(threads) {
  }

  void ArriveAndWait() {
    std::unique_lock lock{mutex_};

    if (--left_ == 0) {
      ++epoch_;
      left_ = threads_;
      lock.unlock();
      new_epoch_.NotifyAll();
    } else {
      size_t epoch = epoch_;
      while (epoch_ == epoch) {
        new_epoch_.Wait(lock);
      }
    }
  }

 private:
  const size_t threads_;
  Mutex mutex_;
  size_t left_;
  size_t epoch_ = 0;
  CondVar new_epoch_;
};

TEST_SUITE(RandomCondVar) {
  TWIST_RANDOMIZE(Event, 5s) {
    twist::ed::std::random_device rd{};
    twist::assist::Choice choice{rd};

    size_t waiters = choice(1, 4);

    twist::assist::Shared<int> data{0};
    Event event;

    twist::test::body::WaitGroup wg;

    wg.Add([&] {
      data.Write(1);
      event.Fire();
    });

    wg.Add(waiters, [&] {
      event.Wait();
      int r = data.Read();
      TWIST_ASSERT(r, "Missing write");
    });

    wg.Join();
  }

  TWIST_RANDOMIZE(Mutex, 5s) {
    twist::ed::std::random_device rd{};
    twist::assist::Choice choice{rd};

    size_t contenders = choice(2, 5);

    Semaphore mutex{1};
    twist::assist::Shared<int> owner;

    twist::test::body::WaitGroup wg;

    wg.Add(contenders, [&](size_t index) {
      mutex.Acquire();
      {
        // Critical section
        owner.Write(index);
      }
      mutex.Release();
    });

    wg.Join();
  }

  TWIST_RANDOMIZE(Semaphore, 5s) {
    twist::ed::std::random_device rd{};
    twist::assist::Choice choice{rd};

    size_t clients = choice(2, 5);
    size_t limit = choice(1, clients);

    Semaphore sema{limit};
    twist::ed::std::atomic<size_t> load{0};

    twist::test::body::WaitGroup wg;

    wg.Add(clients, [&] {
      sema.Acquire();

      size_t l = load.fetch_add(1) + 1;
      TWIST_ASSERT(l <= limit, "Overload");
      load.fetch_sub(1);

      sema.Release();
    });

    wg.Join();
  }

  TWIST_RANDOMIZE(Queue, 5s) {
    twist::ed::std::random_device rd{};
    twist::assist::Choice choice{rd};

    size_t capacity = choice(1, 6);
    size_t consumers = choice(1, 6);
    size_t producers = choice(1, 6);

    Queue queue{capacity};

    twist::test::body::WaitGroup wg;

    twist::ed::std::atomic<size_t> consumed{0};
    twist::ed::std::atomic<size_t> countdown{producers};

    wg.Add(consumers, [&] {
      while (auto item = queue.Pop()) {
        consumed.fetch_add(1);
      }
    });

    wg.Add(producers, [&](size_t index) {
      queue.Push(index);
      if (countdown.fetch_sub(1) == 1) {
        // Last producer
        queue.Close();
      }
    });

    wg.Join();

    TWIST_ASSERT(consumed.load() == producers, "Missing Push-es");
  }

  TWIST_RANDOMIZE(Barrier, 5s) {
    twist::ed::std::random_device rd{};
    twist::assist::Choice choice{rd};

    size_t threads = choice(2, 5);
    size_t iters = choice(2, 5);

    Barrier barrier{threads};
    twist::assist::Shared<size_t> leader{0u};

    twist::test::body::WaitGroup wg;

    wg.Add(threads, [&](size_t me) {
      for (size_t i = 0; i < iters; ++i) {
        size_t l = i % threads;

        barrier.ArriveAndWait();

        if (me == l) {
          leader.Write(me);
        }

        barrier.ArriveAndWait();

        TWIST_ASSERT(leader.Read() == l, "Wrong leader");
      }
    });

    wg.Join();
  }
}

RUN_ALL_TESTS()
