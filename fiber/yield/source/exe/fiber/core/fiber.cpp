#include "fiber.hpp"

#include <cstdlib>
#include <twist/ed/static/thread_local/ptr.hpp>

namespace exe::fiber {

    TWISTED_STATIC_THREAD_LOCAL_PTR(Fiber, current_fiber);

    Fiber::Fiber(Scheduler& scheduler, Body new_task)
        : scheduler_ptr_(&scheduler),
          coroutine_(std::move(new_task)) {}

    void Fiber::Schedule() {
        scheduler_ptr_->Submit([this]() {
            current_fiber = this;
            coroutine_.Resume();
            if (coroutine_.IsDone()) {
                delete this;
            } else {
                Schedule();
            }
        });
    }

    void Fiber::Yield() {
        coroutine_.Suspend();
    }

    Scheduler& Fiber::GetScheduler() {
        return *(Fiber::Self().scheduler_ptr_);
    }

    Fiber& Fiber::Self() {
        return *current_fiber;
    }

}  // namespace exe::fiber
