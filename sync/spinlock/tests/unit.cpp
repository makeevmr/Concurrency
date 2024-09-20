#include "../spinlock.hpp"
#include "../atomic.h"

#include <wheels/test/framework.hpp>

TEST_SUITE(Atomics) {
  SIMPLE_TEST(LoadStore) {
    AtomicStorage s = 0;

    ASSERT_EQ(AtomicLoad(&s), 0);
    AtomicStore(&s, 42);
    ASSERT_EQ(AtomicLoad(&s), 42);
  }

  SIMPLE_TEST(Exchange) {
    AtomicStorage s = 0;

    ASSERT_EQ(AtomicExchange(&s, 7), 0);
    ASSERT_EQ(AtomicLoad(&s), 7);
    ASSERT_EQ(AtomicExchange(&s, 11), 7);
    ASSERT_EQ(AtomicExchange(&s, 42), 11);
  }
}

TEST_SUITE(SpinLock) {
  SIMPLE_TEST(LockUnlock) {
    TASSpinLock spinlock;

    spinlock.Lock();
    spinlock.Unlock();
  }

  SIMPLE_TEST(SequentialLockUnlock) {
    TASSpinLock spinlock;

    spinlock.Lock();
    spinlock.Unlock();

    spinlock.Lock();
    spinlock.Unlock();
  }

  SIMPLE_TEST(TryLock) {
    TASSpinLock spinlock;

    ASSERT_TRUE(spinlock.TryLock());
    ASSERT_FALSE(spinlock.TryLock());
    spinlock.Unlock();
    ASSERT_TRUE(spinlock.TryLock());
    spinlock.Unlock();
    spinlock.Lock();
    ASSERT_FALSE(spinlock.TryLock());
  }
}

RUN_ALL_TESTS()
