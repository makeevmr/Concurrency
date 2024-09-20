#include <wheels/test/framework.hpp>

#include <twist/sim.hpp>

#include "../mutex.hpp"

static_assert(twist::build::IsolatedSim());

TEST_SUITE(MutexSim) {
  SIMPLE_TEST(NoContention) {
    twist::sim::sched::FairScheduler scheduler;
    twist::sim::Simulator simulator{&scheduler};

    auto result = simulator.Run([] {
      Mutex mutex;

      for (size_t i = 0; i < 3; ++i) {
        mutex.Lock();
        mutex.Unlock();
      }
    });

    ASSERT_TRUE(result.Ok());
    ASSERT_TRUE(simulator.FutexCount() == 0);
  }
}

RUN_ALL_TESTS()
