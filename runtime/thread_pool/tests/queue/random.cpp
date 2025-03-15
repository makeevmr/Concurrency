#include <exe/queue.hpp>

#include <course/test/twist.hpp>

#include <twist/test/body/wg.hpp>
#include <twist/test/body/budget.hpp>

#include <twist/assist/assert.hpp>
#include <twist/assist/random.hpp>

#include <twist/ed/std/atomic.hpp>

using namespace std::chrono_literals;  // NOLINT

TEST_SUITE(RandomUnboundedQueue) {
  TWIST_RANDOMIZE(Queue, 5s) {
    twist::ed::std::random_device rd{};
    twist::assist::Choice choice{rd};

    size_t consumers = choice(1, 6);
    size_t producers = choice(1, 6);

    UnboundedBlockingQueue<size_t> queue{};

    twist::ed::std::atomic<size_t> consumed{0};
    twist::ed::std::atomic<size_t> countdown{producers};

    twist::test::body::WaitGroup wg;

    wg.Add(consumers, [&] {
      while (auto item = queue.Pop()) {
        consumed.fetch_add(1);
      }
    });

    wg.Add(producers, [&](size_t index) {
      queue.Push(index);
      if (countdown.fetch_sub(1) == 1) {
        // Last producer
        queue.Close();
      }
    });

    wg.Join();

    TWIST_ASSERT(consumed.load() == producers, "Missing Push-es");
  }
}

RUN_ALL_TESTS()
