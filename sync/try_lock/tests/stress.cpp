#include "../ticket_lock.hpp"

#include <course/test/twist.hpp>
#include <course/test/time_budget.hpp>

#include <twist/test/inject_fault.hpp>
#include <twist/test/wg.hpp>
#include <twist/test/plate.hpp>
#include <twist/test/either.hpp>

#include <twist/ed/wait/spin.hpp>

#include <cstdlib>
#include <vector>
#include <chrono>

using namespace std::chrono_literals;  // NOLINT

TEST_SUITE(TicketTryLock) {
  void StressTest(size_t threads) {
    twist::test::Plate plate;  // Guarded by ticket_lock
    TicketLock ticket_lock;

    twist::test::WaitGroup wg;

    wg.Add(threads, [&] {
      course::test::TimeBudget time_budget;

      while (time_budget) {
        if (twist::test::Either()) {
          ticket_lock.Lock();
          plate.Access();
          ticket_lock.Unlock();
        } else {
          if (ticket_lock.TryLock()) {
            plate.Access();
            ticket_lock.Unlock();
          }
        }
      }
    });

    wg.Join();
  }

  TWIST_STRESS_TEST(Stress2, 5s) {
    StressTest(2);
  }

  TWIST_STRESS_TEST(Stress3, 5s) {
    StressTest(3);
  }

  TWIST_STRESS_TEST(Stress5, 5s) {
    StressTest(5);
  }
}

RUN_ALL_TESTS()
