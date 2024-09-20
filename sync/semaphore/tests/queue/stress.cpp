#include "../../queue.hpp"

#include <course/test/twist.hpp>
#include <course/test/time_budget.hpp>

#include <twist/ed/std/atomic.hpp>

#include <twist/test/wg.hpp>

#include <fmt/core.h>

TEST_SUITE(StressQueue) {
  TWIST_STRESS_TEST(ProducersConsumers, 5s) {
    static const size_t kCapacity = 2;
    static const size_t kProducers = 4;
    static const size_t kConsumers = 4;

    BoundedBlockingQueue<int> queue{kCapacity};

    twist::test::WaitGroup wg;

    twist::ed::std::atomic_size_t takes{0};
    twist::ed::std::atomic_size_t puts{0};

    twist::ed::std::atomic_size_t countdown{kProducers};

    wg.Add(kConsumers, [&] {
      while (true) {
        int v = queue.Take();
        if (v != -1) {
          takes.fetch_add(1);
        } else {
          // Poison pill
          queue.Put(-1);
          break;
        }
      }
    });

    wg.Add(kProducers, [&] {
      course::test::TimeBudget time_budget;

      for (int v = 0; time_budget; ++v) {
        queue.Put(v);
        puts.fetch_add(1);
      }

      if (countdown.fetch_sub(1) == 1) {
        // Last producer
        queue.Put(-1);  // Poison pill
      }
    });

    wg.Join();

    fmt::println("# Put = {}", puts.load());
    fmt::println("# Take = {}", takes.load());

    ASSERT_TRUE(puts.load() == takes.load());
  }
}

RUN_ALL_TESTS()
