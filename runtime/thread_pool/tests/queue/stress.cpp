#include <exe/queue.hpp>

#include <twist/ed/std/atomic.hpp>
#include <twist/ed/std/thread.hpp>

#include <course/test/twist.hpp>
#include <course/test/time_budget.hpp>

#include <twist/test/body/wg.hpp>

#include <fmt/core.h>

#include <chrono>

using namespace std::chrono_literals;  // NOLINT

TEST_SUITE(StressBlockingQueue) {
  TWIST_STRESS_TEST(Ring, 5s) {
    twist::test::body::WaitGroup wg;

    UnboundedBlockingQueue<int> a;
    UnboundedBlockingQueue<int> b;

    twist::ed::std::atomic_bool stop{false};
    twist::ed::std::atomic_uint64_t pops{0};

    for (size_t i = 0; i < 4; ++i) {
      a.Push(i);
    }

    // Workers

    wg.Add(4, [&] {
      while (!stop) {
        auto d = a.Pop();
        ++pops;
        b.Push(*d);
      }
      b.Push(-1);
    });

    wg.Add(4, [&] {
      while (!stop) {
        auto d = b.Pop();
        ++pops;
        a.Push(*d);
      }
      a.Push(-1);
    });

    // Supervisor

    wg.Add([&] {
      twist::ed::std::this_thread::sleep_for(4s);
      stop.store(true);
    });

    wg.Join();

    fmt::println("# Pop = {}", pops.load());
  }
}

RUN_ALL_TESTS()
