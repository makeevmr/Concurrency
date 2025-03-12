#include "../../source/condvar.hpp"
#include "../../source/mutex.hpp"

#include <course/test/twist.hpp>

#include <twist/assist/assert.hpp>
#include <twist/assist/shared.hpp>

#include <twist/test/body/either.hpp>
#include <twist/test/body/wg.hpp>

#include <mutex>

class Event {
 public:
  void Wait() {
    std::unique_lock lock{mutex_};
    while (!ready_.Read()) {
      fire_.Wait(lock);
    }
  }

  void Fire() {
    std::lock_guard guard{mutex_};
    ready_.Write(true);
    fire_.NotifyAll();
  }

 private:
  Mutex mutex_;
  twist::assist::Shared<bool> ready_{false};
  CondVar fire_;
};

TEST_SUITE(ModelCondVar) {
  auto kModelParams = course::test::twist::model::Params{
    .max_preempts = 3
  };

  TWIST_MODEL(Event, kModelParams) {
    twist::assist::Shared<int> data{0};
    Event flag;

    twist::test::body::WaitGroup wg;

    wg.Add([&] {
      data.Write(42);
      flag.Fire();
    });

    wg.Add(2, [&] {
      flag.Wait();
      int d = data.Read();
      TWIST_ASSERT(d == 42, "Unexpected value");
    });

    wg.Join();
  }
}

RUN_ALL_TESTS()
