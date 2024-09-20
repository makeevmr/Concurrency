#include "../../queue.hpp"

#include <course/test/twist.hpp>

#include <twist/ed/std/atomic.hpp>

#include <twist/assist/assert.hpp>
#include <twist/assist/random.hpp>

#include <twist/test/wg.hpp>

TEST_SUITE(RandomQueue) {
  TWIST_RANDOMIZE(ProducersConsumers, 5s) {
    twist::ed::std::random_device rd{};
    twist::assist::Choice choice{rd};

    size_t capacity = choice(1, 6);
    size_t consumers = choice(1, 6);
    size_t producers = choice(1, 6);

    BoundedBlockingQueue<int> queue{capacity};

    twist::test::WaitGroup wg;

    twist::ed::std::atomic<size_t> consumed{0};
    twist::ed::std::atomic<size_t> countdown{producers};

    wg.Add(consumers, [&] {
      while (true) {
        int v = queue.Take();
        if (v != -1) {
          consumed.fetch_add(1);
        } else {
          // Poison pill
          queue.Put(-1);
          break;
        }
      }
    });

    wg.Add(producers, [&](size_t me) {
      queue.Put((int)me);

      if (countdown.fetch_sub(1) == 1) {
        // Last producer
        queue.Put(-1);  // Poison pill
      }
    });

    wg.Join();

    TWIST_ASSERT(consumed.load() == producers, "Missing items");
  }
}

RUN_ALL_TESTS()
