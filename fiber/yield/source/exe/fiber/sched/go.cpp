#include "go.hpp"

#include "../core/fiber.hpp"

namespace exe::fiber {

    void Go(Scheduler& scheduler, Body new_task) {
        (new Fiber(scheduler, std::move(new_task)))->Schedule();
    }

    void Go(Body new_task) {
        (new Fiber(Fiber::GetScheduler(), std::move(new_task)))->Schedule();
    }

}  // namespace exe::fiber
