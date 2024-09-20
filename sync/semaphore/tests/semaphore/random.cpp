#include "../../semaphore.hpp"
#include "../../queue.hpp"

#include <course/test/twist.hpp>

#include <twist/test/wg.hpp>

#include <twist/ed/std/atomic.hpp>

#include <twist/assist/shared.hpp>
#include <twist/assist/assert.hpp>
#include <twist/assist/random.hpp>

TEST_SUITE(RandomSemaphore) {
  TWIST_RANDOMIZE(Resource, 5s) {
    twist::ed::std::random_device rd{};
    twist::assist::Choice choice{rd};

    size_t clients = choice(2, 5);
    size_t limit = choice(1, clients);

    Semaphore sema{limit};
    twist::ed::std::atomic_size_t load{0};

    twist::test::WaitGroup wg;

    wg.Add(clients, [&] {
      sema.Acquire();

      size_t l = load.fetch_add(1) + 1;
      TWIST_ASSERT(l <= limit, "Overload");
      load.fetch_sub(1);

      sema.Release();
    });

    wg.Join();
  }

  TWIST_RANDOMIZE(Mutex, 5s) {
    twist::ed::std::random_device rd{};
    twist::assist::Choice choice{rd};

    size_t contenders = choice(2, 5);

    Semaphore mutex{1};
    twist::assist::Shared<int> owner;

    twist::test::WaitGroup wg;

    wg.Add(contenders, [&](size_t index) {
      mutex.Acquire();
      {
        // Critical section
        owner.Write(index);
      }
      mutex.Release();
    });

    wg.Join();
  }
}

RUN_ALL_TESTS()
