#include "../mutex.hpp"

#include <course/test/twist.hpp>

#include <twist/test/wg.hpp>

#include <twist/assist/shared.hpp>
#include <twist/assist/random.hpp>

TEST_SUITE(MutexRandomCheck) {
  TWIST_RANDOMIZE(Contention, 10s) {
    twist::ed::std::random_device rd{};
    twist::assist::Choice choice{rd};

    const size_t threads = choice(2, 5);

    twist::test::WaitGroup wg;
    twist::assist::Shared<size_t> owner{0u};

    Mutex mutex;

    for (size_t i = 0; i < threads; ++i) {
      const size_t locks = choice(1, 10);

      wg.Add([&, i, locks] {
        for (size_t k = 0; k < locks; ++k) {
          mutex.Lock();
          {
            // Critical section
            owner.Write(i);
          }
          mutex.Unlock();
        }
      });
    }

    wg.Join();
  }
}

RUN_ALL_TESTS()
