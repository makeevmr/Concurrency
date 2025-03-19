#include <exe/promise.hpp>

#include <course/test/twist.hpp>

#include <twist/ed/std/thread.hpp>

#include <twist/assist/assert.hpp>

TEST_SUITE(ModelFuture) {
  TWIST_MODEL(SetValue, {}) {
    Promise<int> p;
    auto f = p.MakeFuture();

    twist::ed::std::thread t([p = std::move(p)]() mutable {
      p.SetValue(1);
    });

    int v = f.Get();
    TWIST_ASSERT(v == 1, "Unexpected value from producer");

    t.join();
  }
}

RUN_ALL_TESTS()
