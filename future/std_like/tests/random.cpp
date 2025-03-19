#include <exe/promise.hpp>

#include <course/test/twist.hpp>

#include <twist/assist/assert.hpp>

#include <twist/test/body/wg.hpp>
#include <twist/test/body/either.hpp>

#include <string>

using namespace std::chrono_literals;  // NOLINT

template <typename T>
void Drop(T) {
}

TEST_SUITE(RandomFuture) {
  TWIST_RANDOMIZE(Rendezvous, 5s) {
    Promise<int> p;
    auto f = p.MakeFuture();
    
    twist::test::body::WaitGroup wg;

    wg.Add([p = std::move(p)]() mutable {
      p.SetValue(17);
    });

    wg.Add([f = std::move(f)]() mutable {
      TWIST_ASSERT(f.Get() == 17, "Unexpected value from producer");
    });

    wg.Join();
  }

  TWIST_RANDOMIZE(ConcurrentRendezvous, 5s) {
    // Contracts

    Promise<std::string> p0;
    auto f0 = p0.MakeFuture();

    Promise<std::string> p1;
    auto f1 = p1.MakeFuture();

    twist::test::body::WaitGroup wg;

    // Producers

    wg.Add([p = std::move(p0)]() mutable {
      p.SetValue("Hello");
    });

    wg.Add([p = std::move(p1)]() mutable {
      p.SetValue("World");
    });

    // Consumers

    wg.Add([f = std::move(f0)]() mutable {
      TWIST_ASSERT(f.Get() == "Hello", "Unexpected value from producer");
    });

    wg.Add([f = std::move(f1)]() mutable {
      TWIST_ASSERT(f.Get() == "World", "Unexpected value from producer");
    });

    wg.Join();
  }
  
  TWIST_RANDOMIZE(SharedState, 5s) {
    Promise<std::string> p;
    Future<std::string> f = p.MakeFuture();

    twist::test::body::WaitGroup wg;

    wg.Add([f = std::move(f)]() mutable {
      if (twist::test::body::Either()) {
        ASSERT_EQ(f.Get(), "Test");
      }
      Drop(std::move(f));
    });

    wg.Add([p = std::move(p)]() mutable {
      p.SetValue("Test");
      Drop(std::move(p));
    });

    wg.Join();
  }
}

RUN_ALL_TESTS()
