#include <exe/thread_pool.hpp>
#include <exe/wait_group.hpp>

#include <fmt/core.h>

int main() {
  ThreadPool pool{/*workers=*/4};
  pool.Start();

  WaitGroup wg;

  pool.Submit([&wg] {
    fmt::println("Running on thread pool");
    wg.Done();
  });

  wg.Wait();

  pool.Stop();

  return 0;
}
