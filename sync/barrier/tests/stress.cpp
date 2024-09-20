#include "../cyclic_barrier.hpp"

#include <course/test/twist.hpp>

#include <twist/test/wg.hpp>

#include <twist/assist/shared.hpp>
#include <twist/assist/assert.hpp>

TEST_SUITE(StressBarrier) {
  TWIST_STRESS_TEST(RotatingLeader, 30s) {
    const size_t kThreads = 5;
    const size_t kWaves = 16384;

    CyclicBarrier barrier{kThreads};
    twist::assist::Shared<size_t> leader{0u};

    twist::test::WaitGroup wg;

    wg.Add(kThreads, [&](size_t me) {
      for (size_t i = 0; i < kWaves; ++i) {
        barrier.ArriveAndWait();

        const size_t l = i % kThreads;

        if (l == me) {
          leader.Write(me);
        }

        barrier.ArriveAndWait();

        TWIST_ASSERT(leader.Read() == l, "Unexpected leader");
      }
    });

    wg.Join();
  }
}

RUN_ALL_TESTS()
