#include "../ticket_lock.hpp"

#include <course/test/twist.hpp>

#include <twist/test/wg.hpp>
#include <twist/test/either.hpp>

#include <twist/assist/shared.hpp>
#include <twist/assist/random.hpp>

TEST_SUITE(TicketTryLock) {
  TWIST_RANDOMIZE(Contention, 10s) {
    twist::ed::std::random_device rd{};
    twist::assist::Choice choice{rd};

    const size_t threads = choice(2, 5);
    const size_t locks = choice(2, 5);

    TicketLock ticket_lock;
    twist::assist::Shared<size_t> owner{0u};

    twist::test::WaitGroup wg;

    wg.Add(threads, [&, locks](size_t me) {
      for (size_t k = 0; k < locks; ++k) {
        if (twist::test::Either()) {
          ticket_lock.Lock();
          owner.Write(me);
          ticket_lock.Unlock();
        } else {
          if (ticket_lock.TryLock()) {
            owner.Write(me);
            ticket_lock.Unlock();
          }
        }
      }
    });

    wg.Join();
  }
}

RUN_ALL_TESTS()
