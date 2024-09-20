#pragma once

extern "C" {
#include "atomic.h"
}

// ~std::atomic<int64_t>

class Atomic {
 public:
  using Value = AtomicValue;

  explicit Atomic(Value init = 0)
      : storage_(init) {
  }

  // Non-copyable
  Atomic(const Atomic&) = delete;
  Atomic& operator=(const Atomic&) = delete;

  // Non-movable
  Atomic(Atomic&&) = delete;
  Atomic& operator=(Atomic&&) = delete;

  void Store(Value value) {
    AtomicStore(&storage_, value);
  }

  Value Load() {
    return AtomicLoad(&storage_);
  }

  Value Exchange(Value new_value) {
    return AtomicExchange(&storage_, new_value);
  }

 private:
  AtomicStorage storage_;
};
