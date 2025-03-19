#include <wheels/test/framework.hpp>

#include <exe/promise.hpp>

#include <wheels/core/compiler.hpp>

#include <string>
#include <thread>

using namespace std::chrono_literals;  // NOLINT

TEST_SUITE(Futures) {
  SIMPLE_TEST(GetValue) {
    Promise<int> p;
    Future<int> f = p.MakeFuture();

    p.SetValue(42);
    ASSERT_EQ(f.Get(), 42);
  }

  struct TestException {
  };

  SIMPLE_TEST(ThrowException) {
    Promise<int> p;
    auto f = p.MakeFuture();

    try {
      throw TestException();
    } catch (...) {
      p.SetException(std::current_exception());
    }

    ASSERT_THROW(f.Get(), TestException)
  }

  SIMPLE_TEST(WaitValue) {
    Promise<std::string> p;
    auto f = p.MakeFuture();

    std::thread producer([p = std::move(p)]() mutable {
      std::this_thread::sleep_for(1s);
      p.SetValue("Hi");
    });

    ASSERT_EQ(f.Get(), "Hi");

    producer.join();
  }

  SIMPLE_TEST(WaitException) {
    Promise<std::string> p;
    auto f = p.MakeFuture();

    std::thread producer([p = std::move(p)]() mutable {
      std::this_thread::sleep_for(1s);
      try {
        throw TestException();
      } catch (...) {
        p.SetException(std::current_exception());
      }
    });

    ASSERT_THROW(f.Get(), TestException);

    producer.join();
  }

  template <typename T>
  void Drop(T) {
  }

  SIMPLE_TEST(DropFuture) {
    Promise<std::string> p;
    auto f = p.MakeFuture();

    Drop(std::move(f));

    std::thread producer([p = std::move(p)]() mutable {
      p.SetValue("Hi");
    });

    producer.join();
  }

  SIMPLE_TEST(DropPromise) {
    Promise<std::string> p;
    auto f = p.MakeFuture();

    p.SetValue("Test");
    Drop(std::move(p));

    ASSERT_EQ(f.Get(), "Test");
  }

  SIMPLE_TEST(Futures) {
    Promise<int> p0;
    Promise<int> p1;
    Promise<int> p2;

    auto f0 = p0.MakeFuture();
    auto f1 = p1.MakeFuture();
    auto f2 = p2.MakeFuture();

    std::thread producer0([&]() {
      std::this_thread::sleep_for(3s);
      p0.SetValue(0);
    });

    std::thread producer1([&]() {
      std::this_thread::sleep_for(1s);
      p1.SetValue(1);
    });

    std::thread producer2([&]() {
      std::this_thread::sleep_for(2s);
      p2.SetValue(2);
    });

    ASSERT_EQ(f0.Get(), 0);
    ASSERT_EQ(f1.Get(), 1);
    ASSERT_EQ(f2.Get(), 2);

    producer0.join();
    producer1.join();
    producer2.join();
  }

  SIMPLE_TEST(Valid) {
    Promise<int> p;
    auto f = p.MakeFuture();

    ASSERT_TRUE(f.Valid());

    p.SetValue(4);
    f.Get();

    ASSERT_FALSE(f.Valid());
  }

  SIMPLE_TEST(JustPromise) {
    Promise<int> p;

    WHEELS_UNUSED(p);
  }

  SIMPLE_TEST(SetValueBeforeMakeFuture) {
    Promise<int> p;

    p.SetValue(13);

    auto f = p.MakeFuture();
    int v = f.Get();

    ASSERT_EQ(v, 13);
  }

  SIMPLE_TEST(MoveAssignment) {
    Promise<int> p1;
    auto f1 = p1.MakeFuture();

    Promise<int> p2;
    auto f2 = p2.MakeFuture();

    f1 = std::move(f2);

    p1.SetValue(1);
    p2.SetValue(2);

    auto v = f1.Get();

    ASSERT_EQ(v, 2);
  }

  SIMPLE_TEST(BrokenPromise) {
    Promise<int> p;
    auto f = p.MakeFuture();

    {
      // Abandon shared state
      // https://en.cppreference.com/w/cpp/thread/promise

      auto _ = std::move(p);
      WHEELS_UNUSED(_);
    }

    ASSERT_THROW(f.Get(), BrokenPromiseError);
  }

  SIMPLE_TEST(PromiseAlreadySatisfied) {
    // https://en.cppreference.com/w/cpp/thread/promise/set_value

    {
      Promise<int> p;
      auto f = p.MakeFuture();

      p.SetValue(1);
      ASSERT_THROW(p.SetValue(2), PromiseAlreadySatisfiedError);

      f.Get();
    }

    {
      struct TestError {};

      Promise<int> p;
      auto f = p.MakeFuture();

      p.SetValue(1);
      ASSERT_THROW(p.SetException(std::make_exception_ptr(TestError{})),
                   PromiseAlreadySatisfiedError);

      f.Get();
    }
  }

  SIMPLE_TEST(GetNoState) {
    // See Note on https://en.cppreference.com/w/cpp/thread/future/get

    Promise<int> p;
    auto f = p.MakeFuture();

    p.SetValue(1);
    f.Get();
    ASSERT_THROW(f.Get(), NoStateError);
  }

  SIMPLE_TEST(SetNoState) {
    // https://en.cppreference.com/w/cpp/thread/promise/set_value

    {
      Promise<int> p;

      {
        auto _ = std::move(p);
        WHEELS_UNUSED(_);
      }

      ASSERT_THROW(p.SetValue(1), NoStateError);
    }

    {
      Promise<int> p;

      {
        auto _ = std::move(p);
        WHEELS_UNUSED(_);
      }

      struct TestError{};

      ASSERT_THROW(p.SetException(std::make_exception_ptr(TestError{})),
                   NoStateError);
    }
  }
}

RUN_ALL_TESTS()
