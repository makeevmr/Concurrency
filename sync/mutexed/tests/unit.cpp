#include "../mutexed.hpp"

#include <wheels/test/framework.hpp>

#include <twist/build.hpp>

#include <chrono>
#include <set>
#include <string>
#include <thread>
#include <vector>

using namespace std::chrono_literals;

static_assert(twist::build::Plain());

TEST_SUITE(Mutexed) {
  SIMPLE_TEST(Vector) {
    Mutexed<std::vector<int>> ints;

    {
      auto owner_ref = ints.Acquire();
      ASSERT_TRUE(owner_ref->empty());
    }

    {
      auto owner_ref = ints.Acquire();

      owner_ref->push_back(42);
      ASSERT_EQ(owner_ref->front(), 42);
      ASSERT_EQ(owner_ref->at(0), 42);
      ASSERT_EQ(owner_ref->size(), 1);
    }

    {
      auto owner_ref = ints.Acquire();
      owner_ref->push_back(99);
      ASSERT_EQ(owner_ref->size(), 2);
    }
  }

  SIMPLE_TEST(Set) {
    Mutexed<std::set<std::string>> strings;

    {
      auto owner_ref = strings.Acquire();
      owner_ref->insert("Hello");
      owner_ref->insert("World");
      owner_ref->insert("!");
    }

    ASSERT_EQ(Acquire(strings)->size(), 3);
  }

  class Counter {
   public:
    void Increment() {
      size_t value = value_;
      std::this_thread::sleep_for(1s);
      value_ = value + 1;
    }

    size_t Value() const {
      return value_;
    }

   private:
    size_t value_{0};
  };

  SIMPLE_TEST(Counter) {
    Mutexed<Counter> counter;

    std::thread t1([&] {
      Acquire(counter)->Increment();
    });
    std::thread t2([&] {
      Acquire(counter)->Increment();
    });

    t1.join();
    t2.join();

    ASSERT_EQ(Acquire(counter)->Value(), 2);
  }

  SIMPLE_TEST(Ctor) {
    Mutexed<std::string> str(5, '!');
    ASSERT_EQ(Acquire(str)->length(), 5);
  }

  class Widget {
   public:
    int Foo() {
      return 2;
    }

    int Bar() {
      return 3;
    }
  };

  SIMPLE_TEST(DereferenceWidget) {
    Mutexed<Widget> w;

    {
      auto owner_ref = w.Acquire();

      int a = (*owner_ref).Foo();
      ASSERT_EQ(a, 2);

      int b = (*owner_ref).Bar();
      ASSERT_EQ(b, 3);
    }
  }

  SIMPLE_TEST(DereferenceInt) {
    Mutexed<int> counter{0};

    {
      auto writer = counter.Acquire();

      *writer += 1;
      *writer += 2;

      int val = *writer;

      ASSERT_EQ(val, 3);
    }
  }
}

RUN_ALL_TESTS()
