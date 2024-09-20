#include "../../queue.hpp"

#include <wheels/test/framework.hpp>

#include <atomic>
#include <chrono>
#include <deque>
#include <random>
#include <string>
#include <thread>

using namespace std::chrono_literals;  // NOLINT

TEST_SUITE(BoundedBlockingQueue) {
  SIMPLE_TEST(PutThenTake) {
    BoundedBlockingQueue<int> queue{1};
    queue.Put(42);
    ASSERT_EQ(queue.Take(), 42);
  }

  struct MoveOnly {
    MoveOnly() = default;

    MoveOnly(const MoveOnly& that) = delete;
    MoveOnly& operator=(const MoveOnly& that) = delete;

    MoveOnly(MoveOnly&& that) = default;
    MoveOnly& operator=(MoveOnly&& that) = default;
  };

  SIMPLE_TEST(MoveOnly) {
    BoundedBlockingQueue<MoveOnly> queue{1};

    queue.Put(MoveOnly{});
    queue.Take();
  }

  SIMPLE_TEST(Buffer) {
    BoundedBlockingQueue<std::string> queue{2};

    queue.Put("hello");
    queue.Put("world");

    ASSERT_EQ(queue.Take(), "hello");
    ASSERT_EQ(queue.Take(), "world");
  }

  SIMPLE_TEST(FifoSmall) {
    BoundedBlockingQueue<std::string> queue{2};

    std::thread producer([&queue] {
      queue.Put("hello");
      queue.Put("world");
      queue.Put("!");
    });

    ASSERT_EQ(queue.Take(), "hello");
    ASSERT_EQ(queue.Take(), "world");
    ASSERT_EQ(queue.Take(), "!");

    producer.join();
  }

  SIMPLE_TEST(Fifo) {
    BoundedBlockingQueue<int> queue{3};

    static const int kItems = 1024;

    std::thread producer([&] {
      for (int i = 0; i < kItems; ++i) {
        queue.Put(i);
      }
      queue.Put(-1);  // Poison pill
    });

    // Consumer

    for (int i = 0; i < kItems; ++i) {
      ASSERT_EQ(queue.Take(), i);
    }
    ASSERT_EQ(queue.Take(), -1);

    producer.join();
  }

  SIMPLE_TEST(Capacity) {
    BoundedBlockingQueue<int> queue{3};
    std::atomic<size_t> send_count{0};

    std::thread producer([&] {
      for (size_t i = 0; i < 100; ++i) {
        queue.Put(i);
        send_count.store(i);
      }
      queue.Put(-1);
    });

    std::this_thread::sleep_for(100ms);

    ASSERT_TRUE(send_count.load() <= 3);

    for (size_t i = 0; i < 14; ++i) {
      (void)queue.Take();
    }

    std::this_thread::sleep_for(100ms);

    ASSERT_TRUE(send_count.load() <= 17);

    while (queue.Take() != -1) {
      // Pass
    }

    producer.join();
  }

  SIMPLE_TEST(Pill) {
    static const size_t kThreads = 10;
    BoundedBlockingQueue<int> queue{1};

    std::vector<std::thread> threads;

    std::mt19937 twister;

    for (size_t i = 0; i < kThreads; ++i) {
      auto delay = 1ms * (twister() % 1000);

      threads.emplace_back([&, delay] {
        std::this_thread::sleep_for(delay);

        ASSERT_EQ(queue.Take(), -1);
        queue.Put(-1);
      });
    }

    queue.Put(-1);

    for (auto& t : threads) {
      t.join();
    }
  }
}

RUN_ALL_TESTS()
