#include "../cyclic_barrier.hpp"

#include <course/test/twist.hpp>

#include <twist/assist/assert.hpp>
#include <twist/assist/random.hpp>
#include <twist/assist/shared.hpp>

#include <twist/test/wg.hpp>

TEST_SUITE(RandomBarrier) {
  TWIST_RANDOMIZE(Waves, 5s) {
    twist::ed::std::random_device rd{};
    twist::assist::Choice choice{rd};

    const size_t threads = choice(1, 4);
    const size_t waves = choice(1, 4);

    CyclicBarrier barrier{threads};

    twist::test::WaitGroup wg;

    wg.Add(threads, [&] {
      for (size_t i = 0; i < waves; ++i) {
        barrier.ArriveAndWait();
      }
    });

    wg.Join();
  }

  TWIST_RANDOMIZE(RotatingLeader, 5s) {
    twist::ed::std::random_device rd{};
    twist::assist::Choice choice{rd};

    const size_t threads = choice(1, 4);
    const size_t waves = choice(1, 17);

    CyclicBarrier barrier{threads};
    twist::assist::Shared<size_t> leader{0u};

    twist::test::WaitGroup wg;

    wg.Add(threads, [&](size_t me) {
      for (size_t i = 0; i < waves; ++i) {
        barrier.ArriveAndWait();

        const size_t l = i % threads;

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
