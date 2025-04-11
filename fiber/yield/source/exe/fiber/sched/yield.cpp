#include "yield.hpp"

#include "../core/fiber.hpp"

namespace exe::fiber {

    void Yield() {
        Fiber::Self().Yield();
    }

}  // namespace exe::fiber
