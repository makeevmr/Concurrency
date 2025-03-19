#include <exe/promise.hpp>

#include <fmt/core.h>

#include <string>
#include <thread>

int main() {
  // Contract
  Promise<std::string> p;
  auto f = p.MakeFuture();

  // Producer
  std::thread producer([p = std::move(p)]() mutable {
    p.SetValue("Hello");
  });

  // Consumer
  auto s = f.Get();
  fmt::println("Message = {}", s);

  producer.join();

  return 0;
}
