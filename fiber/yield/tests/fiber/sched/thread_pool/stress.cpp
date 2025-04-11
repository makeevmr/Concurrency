#include <exe/runtime/thread_pool.hpp>
#include <exe/thread/wait_group.hpp>

#include <exe/fiber/sched/go.hpp>
#include <exe/fiber/sched/yield.hpp>

#include <course/test/twist.hpp>
#include <course/test/time_budget.hpp>

using namespace exe;  // NOLINT

TEST_SUITE(StressFibers) {
  TWIST_STRESS_TEST(Yield, 5s) {
    runtime::ThreadPool scheduler{4};
    scheduler.Start();

    const size_t kFibers = 17;

    thread::WaitGroup wg;

    for (size_t i = 0; i < kFibers; ++i) {
      wg.Add(1);
      fiber::Go(scheduler, [&] {
        course::test::TimeBudget time_budget;
        while (time_budget) {
          fiber::Yield();
        }

        wg.Done();
      });
    }

    wg.Wait();

    scheduler.Stop();
  }
}

RUN_ALL_TESTS()
