#include "../mutexed.hpp"

#include <course/test/twist.hpp>
#include <course/test/time_budget.hpp>

#include <twist/test/wg.hpp>
#include <twist/test/plate.hpp>

TEST_SUITE(Mutexed) {
  TWIST_STRESS_TEST(Contention, 3s) {
    static const size_t kThreads = 3;

    // Set of hungry threads
    twist::test::WaitGroup wg;

    // Plate shared between threads
    Mutexed<twist::test::Plate> plate;

    wg.Add(kThreads, [&] {
      course::test::TimeBudget budget;

      while (budget) {
        Acquire(plate)->Access();
      }
    });

    wg.Join();
  }
}

RUN_ALL_TESTS()
