#include "../../semaphore.hpp"

#include <course/test/twist.hpp>
#include <course/test/time_budget.hpp>

#include <twist/assist/shared.hpp>

#include <twist/test/wg.hpp>

TEST_SUITE(StressSemaphore) {
  TWIST_STRESS_TEST(Mutex, 5s) {
    Semaphore mutex{1};
    twist::assist::Shared<size_t> owner{0u};

    twist::test::WaitGroup wg;

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
