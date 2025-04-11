#pragma once

#include "body.hpp"
#include "coroutine.hpp"
#include "scheduler.hpp"

namespace exe::fiber {

    // Fiber = Stackful coroutine x Scheduler
    class Fiber {
    public:
        Fiber(Scheduler&, Body);

        void Schedule();

        void Yield();

        static Scheduler& GetScheduler();

        static Fiber& Self();

    private:
        Scheduler* scheduler_ptr_;
        Coroutine coroutine_;
    };

}  // namespace exe::fiber
