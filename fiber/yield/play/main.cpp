#include <exe/runtime/thread_pool.hpp>

#include <exe/fiber/sched/go.hpp>
#include <exe/fiber/sched/yield.hpp>

#include <exe/thread/wait_group.hpp>

#include <exe/util/defer.hpp>

#include <fmt/core.h>

using namespace exe;  // NOLINT

int main() {
  runtime::ThreadPool scheduler{4};
  scheduler.Start();

  thread::WaitGroup wg;

  for (size_t i = 0; i < 128; ++i) {
    wg.Add(1);

    fiber::Go(scheduler, [&] {
      Defer defer([&] {
        wg.Done();
      });

      for (size_t j = 0; j < 1024; ++j) {
        fiber::Yield();
      }
    });
  }

  wg.Wait();

  scheduler.Stop();

  return 0;
}
