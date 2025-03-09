#include "../source/mutex.hpp"

#include <course/test/twist.hpp>
#include <course/test/time_budget.hpp>

#include <twist/test/body/wg.hpp>
#include <twist/test/body/plate.hpp>

#include <chrono>

using namespace std::chrono_literals;

TEST_SUITE(MutexStress) {
  TWIST_STRESS_TEST(Contention, 5s) {
    static const size_t kThreads = 4;

    twist::test::body::Plate plate;  // Guarded by mutex
    Mutex mutex;

    twist::test::body::WaitGroup wg;

    wg.Add(kThreads, [&] {
      course::test::TimeBudget budget;

      while (budget) {
        mutex.Lock();
        {
          // Critical section
          plate.Access();
        }
        mutex.Unlock();
      }
    });

    wg.Join();

    std::cout << "Critical sections: " << plate.AccessCount() << std::endl;
  }
}

RUN_ALL_TESTS()
