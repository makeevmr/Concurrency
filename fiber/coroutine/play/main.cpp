#include "../source/coroutine.hpp"

#include <fmt/core.h>

#include <cassert>

int main() {
  Coroutine coro([](auto self) {
    fmt::println("Step 2");

    self.Suspend();

    fmt::println("Step 4");
  });

  fmt::println("Step 1");

  coro.Resume();

  fmt::println("Step 3");

  coro.Resume();

  assert(coro.IsDone());
}
