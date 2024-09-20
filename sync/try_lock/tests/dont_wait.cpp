#include "../ticket_lock.hpp"

#include <wheels/test/framework.hpp>

#include <course/test/twist.hpp>

#include <twist/sim.hpp>

#include <twist/ed/std/thread.hpp>

#include <twist/assist/assert.hpp>

#include <fmt/core.h>

#include <chrono>

using namespace std::chrono_literals;

static_assert(twist::build::IsolatedSim());

TEST_SUITE(TicketTryLock) {
  TWIST_RANDOMIZE(DontWaitInTryLock, 3s) {
    TicketLock ticket_lock;

    twist::ed::std::thread locker([&] {
      ticket_lock.Lock();
      twist::ed::std::this_thread::sleep_for(1s);
      ticket_lock.Unlock();
    });

    {
      // Don't call Lock from TryLock
      if (ticket_lock.TryLock()) {
        ticket_lock.Unlock();
      }
    }

    locker.join();

    {
      size_t yield = ::twist::sim::Simulator::Current()->YieldCount();
      TWIST_ASSERT(yield < 37, "Don't wait in TryLock");
    }
  }
}

RUN_ALL_TESTS()
