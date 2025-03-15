#include <exe/queue.hpp>

#include <wheels/test/framework.hpp>

#include <course/test/cpu.hpp>

#include <string>
#include <thread>

template <typename T>
using Queue = UnboundedBlockingQueue<T>;

TEST_SUITE(BlockingQueue) {
  SIMPLE_TEST(JustWorks) {
    Queue<int> queue;

    queue.Push(7);

    auto value = queue.Pop();
    ASSERT_TRUE(value);
    ASSERT_EQ(*value, 7);

    queue.Close();
    ASSERT_FALSE(queue.Pop());
  }

  SIMPLE_TEST(Fifo) {
    Queue<int> queue;
    queue.Push(1);
    queue.Push(2);
    queue.Push(3);

    ASSERT_EQ(*queue.Pop(), 1);
    ASSERT_EQ(*queue.Pop(), 2);
    ASSERT_EQ(*queue.Pop(), 3);
  }

  SIMPLE_TEST(Close) {
    Queue<std::string> queue;

    queue.Push("Hello");
    queue.Push(",");
    queue.Push("World");

    queue.Close();

    ASSERT_EQ(*queue.Pop(), "Hello");
    ASSERT_EQ(*queue.Pop(), ",");
    ASSERT_EQ(*queue.Pop(), "World");
    ASSERT_FALSE(queue.Pop());
  }

  struct MoveOnly {
    MoveOnly() = default;

    MoveOnly(const MoveOnly& that) = delete;
    MoveOnly& operator=(const MoveOnly& that) = delete;

    MoveOnly(MoveOnly&& that) = default;
    MoveOnly& operator=(MoveOnly&& that) = default;
  };

  SIMPLE_TEST(MoveOnly) {
    Queue<MoveOnly> queue;

    queue.Push(MoveOnly{});
    ASSERT_TRUE(queue.Pop().has_value());
  }

  SIMPLE_TEST(BlockingPop) {
    Queue<int> queue;

    std::thread producer([&]() {
      std::this_thread::sleep_for(1s);
      queue.Push(7);
    });

    course::test::ThreadCPUTimer thread_cpu_timer;

    auto value = queue.Pop();

    auto spent = thread_cpu_timer.Spent();

    ASSERT_TRUE(value);
    ASSERT_EQ(*value, 7);
    ASSERT_TRUE(spent < 100ms);

    producer.join();
  }

  SIMPLE_TEST(BlockingPop2) {
    Queue<int> queue;

    std::thread producer([&]() {
      std::this_thread::sleep_for(1s);
      queue.Close();
    });

    course::test::ThreadCPUTimer thread_cpu_timer;

    auto value = queue.Pop();

    auto spent = thread_cpu_timer.Spent();

    ASSERT_FALSE(value);
    ASSERT_TRUE(spent < 100ms);

    producer.join();
  }

  SIMPLE_TEST(UnblockConsumers) {
    Queue<int> queue;

    // Consumers

    std::thread consumer1([&]() {
      queue.Pop();
    });

    std::thread consumer2([&]() {
      queue.Pop();
    });

    // Producer
    std::this_thread::sleep_for(100ms);
    queue.Close();

    consumer1.join();
    consumer2.join();
  }

  SIMPLE_TEST(ProducerConsumer) {
    Queue<int> queue;

    course::test::ProcessCPUTimer process_cpu_timer;

    std::thread producer([&]() {
      // Producer
      for (int i = 0; i < 10; ++i) {
        queue.Push(i);
        std::this_thread::sleep_for(100ms);
      }
      queue.Close();
    });

    // Consumer

    for (int i = 0; i < 10; ++i) {
      auto value = queue.Pop();
      ASSERT_TRUE(value);
      ASSERT_EQ(*value, i);
    }

    ASSERT_FALSE(queue.Pop());

    producer.join();

    ASSERT_TRUE(process_cpu_timer.Spent() < 100ms);
  }
}

RUN_ALL_TESTS()
