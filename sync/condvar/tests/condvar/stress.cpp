#include "../../source/condvar.hpp"
#include "../../source/mutex.hpp"

#include <course/test/twist.hpp>
#include <course/test/time_budget.hpp>

#include <twist/assist/shared.hpp>

#include <twist/test/body/either.hpp>
#include <twist/test/body/wg.hpp>

#include <mutex>

class Semaphore {
 public:
  explicit Semaphore(size_t init = 0)
      : count_(init) {
  }

  void Acquire() {
    std::unique_lock lock{mutex_};
    while (count_ == 0) {
      release_.Wait(lock);
    }
    --count_;
  }

  void Release() {
    std::unique_lock lock{mutex_};
    ++count_;

    if (twist::test::body::Either()) {
      lock.unlock();
    }

    release_.NotifyOne();
  }

 private:
  size_t count_;
  Mutex mutex_;
  CondVar release_;
};

TEST_SUITE(StressCondVar) {
  TWIST_STRESS_TEST(Semaphore, 5s) {
    Semaphore mutex{1};
    twist::assist::Shared<size_t> owner{0u};

    twist::test::body::WaitGroup wg;

    wg.Add(4, [&](size_t index) {
      course::test::TimeBudget time_budget;

      while (time_budget) {
        mutex.Acquire();
        {
          owner.Write(index);
        }
        mutex.Release();
      }
    });

    wg.Join();
  }
}

RUN_ALL_TESTS()
