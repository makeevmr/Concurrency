#include "../../semaphore.hpp"

#include <wheels/test/framework.hpp>

#include <atomic>
#include <chrono>
#include <deque>
#include <random>
#include <string>
#include <thread>

using namespace std::chrono_literals;

TEST_SUITE(Semaphore) {
  SIMPLE_TEST(NonBlocking) {
    Semaphore semaphore{2};

    semaphore.Acquire();  // -1
    semaphore.Release();  // +1

    semaphore.Acquire();  // -1
    semaphore.Acquire();  // -1
    semaphore.Release();  // +1
    semaphore.Release();  // +1
  }

  SIMPLE_TEST(Blocking) {
    Semaphore semaphore{0};

    bool touched = false;

    std::thread touch([&] {
      semaphore.Acquire();
      touched = true;
    });

    std::this_thread::sleep_for(250ms);

    ASSERT_FALSE(touched);

    semaphore.Release();
    touch.join();

    ASSERT_TRUE(touched);
  }

  SIMPLE_TEST(PingPong) {
    Semaphore my{1};
    Semaphore that{0};

    int step = 0;

    std::thread opponent([&] {
      that.Acquire();
      ASSERT_EQ(step, 1);
      step = 0;
      my.Release();
    });

    my.Acquire();
    ASSERT_EQ(step, 0);
    step = 1;
    that.Release();

    my.Acquire();
    ASSERT_TRUE(step == 0);

    opponent.join();
  }
}

RUN_ALL_TESTS()
