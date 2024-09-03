#include <wheels/test/framework.hpp>

#include "../lock_guard.hpp"

#include <mutex>
#include <utility>

TEST_SUITE(LockGuard) {
  class TestMutex {
   public:
    void Lock() {
      ASSERT_FALSE(locked_);
      locked_ = true;
    }

    bool TryLock() {
      return !std::exchange(locked_, true);
    }

    void Unlock() {
      ASSERT_TRUE(locked_);
      locked_ = false;
    }

    // Lockable

    void lock() {  // NOLINT
      Lock();
    }

    bool try_lock() {  // NOLINT
      return TryLock();
    }

    void unlock() {  // NOLINT
      Unlock();
    }

   private:
    bool locked_{false};
  };

  SIMPLE_TEST(JustWorks) {
    TestMutex mutex;

    {
      LockGuard guard{mutex};

      ASSERT_FALSE(mutex.try_lock());
    }

    {
      mutex.lock();

      mutex.unlock();
    }
  }

  SIMPLE_TEST(Unwind) {
    TestMutex mutex;

    try {
      {
        LockGuard guard{mutex};


        ASSERT_FALSE(mutex.try_lock());

        throw 1;
      }
    } catch (int) {
      // Ignore
    }

    {
      mutex.lock();
      mutex.unlock();
    }
  }

  SIMPLE_TEST(StdMutex) {
    std::mutex mutex;

    {
      LockGuard guard{mutex};
    }

    {
      mutex.lock();
      mutex.unlock();
    }
  }
}

RUN_ALL_TESTS()
