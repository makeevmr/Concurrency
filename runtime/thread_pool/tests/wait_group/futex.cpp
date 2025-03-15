#include <exe/wait_group.hpp>

#include <wheels/test/framework.hpp>

#include <course/test/twist.hpp>

#include <twist/sim.hpp>
#include <twist/build.hpp>

#include <twist/ed/std/thread.hpp>
#include <twist/assist/assert.hpp>

static_assert(twist::build::IsolatedSim());

TEST_SUITE(WaitGroupFutex) {
  SIMPLE_TEST(Sequential) {
    twist::sim::sched::FairScheduler scheduler;
    twist::sim::Simulator simulator{&scheduler};

    auto result = simulator.Run([] {
      WaitGroup wg;

      static const size_t kIters = 3;
      static const size_t kWorkPerIter = 17;

      for (size_t k = 0; k < kIters; ++k) {
        // 0

        for (size_t i = 0; i < kWorkPerIter; ++i) {
          wg.Add(1);
        }

        for (size_t i = 0; i < kWorkPerIter; ++i) {
          wg.Done();
        }

        wg.Wait();
      }
    });

    ASSERT_TRUE(result.Ok());
    ASSERT_TRUE(simulator.FutexCount() == 0);
  }

  TWIST_RANDOMIZE(ConcurrentThenSequential, 3s) {
    WaitGroup wg;

    {
      // Wait #1

      wg.Add(1);
      twist::ed::std::thread t([&wg] {
        wg.Done();
      });

      for (size_t i = 1; i < 5; ++i) {
        wg.Add(1);
      }

      for (size_t i = 1; i < 5; ++i) {
        wg.Done();
      }

      wg.Wait();

      t.join();
    }

    {
      // Wait #2

      size_t wakes_start = twist::sim::FutexWakeSystemCallCount();

      for (size_t i = 0; i < 11; ++i) {
        wg.Add(1);
      }

      for (size_t i = 0; i < 11; ++i) {
        wg.Done();
      }

      size_t wakes = twist::sim::FutexWakeSystemCallCount() - wakes_start;

      TWIST_ASSERT(wakes == 0, "Extra FutexWake system calls");

      wg.Wait();
    }
  }
}

RUN_ALL_TESTS()
