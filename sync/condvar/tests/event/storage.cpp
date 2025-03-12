#include "../../source/mutex.hpp"
#include "../../source/condvar.hpp"

#include <course/test/twist.hpp>

#include <twist/ed/std/thread.hpp>

// std::unique_lock / std::lock_guard
#include <mutex>

class Event {
 public:
  void Wait() {
    std::unique_lock locker(mutex_);
    while (!ready_) {
      fire_.Wait(locker);
    }
  }

  void Fire() {
    std::lock_guard guard(mutex_);
    ready_ = true;
    fire_.NotifyOne();
  }

 private:
  bool ready_{false};
  Mutex mutex_;
  CondVar fire_;
};

TEST_SUITE(Event) {
  /*
   * Equivalent to:
   *
   * ThreadPool pool{4};
   * pool.Start();
   *
   * {
   *   Event event;
   *
   *   pool.Submit([&event] {
   *     event.Fire();
   *   });
   *
   *   event.Wait();
   * }  // ~Event
   *
   * pool.Stop();
   */

  TWIST_RANDOMIZE(Storage, 5s) {
    auto event = new Event{};

    twist::ed::std::thread t([event] {
      event->Fire();
    });

    event->Wait();
    delete event;

    t.join();
  }
}

RUN_ALL_TESTS()
