#include <exe/wait_group.hpp>

#include <course/test/twist.hpp>

#include <twist/ed/std/thread.hpp>

#include <twist/assist/shared.hpp>
#include <twist/assist/assert.hpp>

TEST_SUITE(WaitGroup) {
  const auto kParams =
      course::test::twist::model::Params{.max_preempts = 4};

  TWIST_MODEL(AddAddWait, kParams) {
    WaitGroup wg;

    twist::assist::Shared<bool> f1{false};
    twist::assist::Shared<bool> f2{false};

    wg.Add(1);

    twist::ed::std::thread t1([&] {
      f1.Write(true);
      wg.Done();
    });

    wg.Add(1);

    twist::ed::std::thread t2([&] {
      f2.Write(true);
      wg.Done();
    });

    twist::ed::std::thread t3([&] {
      wg.Wait();

      bool r1 = f1.Read();
      bool r2 = f2.Read();

      TWIST_ASSERT(r1 && r2, "Unfinished work");
    });

    t1.join();
    t2.join();
    t3.join();
  }

  TWIST_MODEL(AddWaitAdd, kParams) {
    WaitGroup wg;

    twist::assist::Shared<bool> f1{false};
    twist::assist::Shared<bool> f2{false};

    wg.Add(1);

    twist::ed::std::thread t2;

    twist::ed::std::thread t1([&] {
      wg.Add(1);

      t2 = twist::ed::std::thread([&] {
        f2.Write(true);
        wg.Done();
      });

      f1.Write(true);
      wg.Done();
    });

    twist::ed::std::thread t3([&] {
      wg.Wait();

      bool r1 = f1.Read();
      bool r2 = f2.Read();

      TWIST_ASSERT(r1 && r2, "Unfinished work");
    });

    t1.join();
    t2.join();
    t3.join();
  }
}

RUN_ALL_TESTS()
