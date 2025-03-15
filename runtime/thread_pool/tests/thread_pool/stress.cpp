#include <exe/thread_pool.hpp>
#include <exe/wait_group.hpp>

#include <twist/ed/std/atomic.hpp>

#include <twist/assist/assert.hpp>

#include <course/test/twist.hpp>
#include <course/test/time_budget.hpp>

TEST_SUITE(StressThreadPool) {
  TWIST_STRESS_TEST(SubmitAndWait, 5s) {
    ThreadPool pool{4};
    pool.Start();

    course::test::TimeBudget time_budget;

    for (size_t iter = 0; time_budget; ++iter) {
      size_t todo = 1 + iter % 11;
      twist::ed::std::atomic_size_t done{0};

      WaitGroup wg;
      for (size_t i = 0; i < todo; ++i) {
        wg.Add(1);
        pool.Submit([&] {
          ++done;
          wg.Done();
        });
      }

      wg.Wait();

      TWIST_ASSERT(done.load() == todo, "Missing tasks");
    }

    pool.Stop();
  }
}

RUN_ALL_TESTS()
