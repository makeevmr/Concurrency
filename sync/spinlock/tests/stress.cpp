#include "../spinlock.hpp"

#include <course/test/twist.hpp>
#include <course/test/time_budget.hpp>

#include <twist/test/wg.hpp>
#include <twist/test/plate.hpp>

#include <twist/ed/wait/spin.hpp>

#include <chrono>

////////////////////////////////////////////////////////////////////////////////

using namespace std::chrono_literals;

////////////////////////////////////////////////////////////////////////////////

TEST_SUITE(SpinLock) {
  void StressTest(size_t lockers, size_t try_lockers) {
    twist::test::Plate plate;  // Guarded by spinlock
    TASSpinLock spinlock;

    twist::test::WaitGroup wg;

    wg.Add(lockers, [&] {
      course::test::TimeBudget budget;

      while (budget) {
        spinlock.Lock();
        plate.Access();
        spinlock.Unlock();
      }
    });

    wg.Add(try_lockers, [&] {
      course::test::TimeBudget budget;

      while (budget) {
        twist::ed::SpinWait spin_wait;
        while (!spinlock.TryLock()) {
          spin_wait();
        }
        plate.Access();
        spinlock.Unlock();
      }
    });

    wg.Join();

    std::cout << "Critical sections: " << plate.AccessCount() << std::endl;
  }

  TWIST_STRESS_TEST(Stress1, 5s) {
    StressTest(3, 0);
  }

  TWIST_STRESS_TEST(Stress2, 5s) {
    StressTest(0, 3);
  }

  TWIST_STRESS_TEST(Stress3, 5s) {
    StressTest(3, 3);
  }

  TWIST_STRESS_TEST(Stress4, 10s) {
    StressTest(5, 5);
  }

  TWIST_STRESS_TEST(Stress5, 10s) {
    StressTest(10, 10);
  }
}

RUN_ALL_TESTS()
