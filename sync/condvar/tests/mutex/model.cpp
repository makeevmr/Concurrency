#include "../../source/mutex.hpp"

#include <course/test/twist.hpp>

#include <twist/test/body/wg.hpp>
#include <twist/assist/shared.hpp>

TEST_SUITE(MutexModel) {
  TWIST_MODEL(Duo, {}) {
    twist::test::body::WaitGroup wg;
    twist::assist::Shared<size_t> owner{0u};

    Mutex mutex;

    wg.Add(2, [&](size_t me) {
      mutex.Lock();
      {
        // Critical section
        owner.Write(me);
      }
      mutex.Unlock();
    });

    wg.Join();
  }

  const auto kTrioParams =
      course::test::twist::model::Params{
          .max_preempts = 5,
          .max_steps = 128};

  TWIST_MODEL(Trio, kTrioParams) {
    twist::test::body::WaitGroup wg;
    twist::assist::Shared<size_t> owner{0u};

    Mutex mutex;

    wg.Add(3, [&](size_t me) {
      for (size_t i = 0; i < 2; ++i) {
        mutex.Lock();
        {
          // Critical section
          owner.Write(me);
        }
        mutex.Unlock();
      }
    });

    wg.Join();
  }
}

RUN_ALL_TESTS()
