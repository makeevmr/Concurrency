#pragma once

#include "body.hpp"
#include "stack.hpp"

#include <twist/ed/sure/context.hpp>

namespace exe::fiber {

    class Coroutine : public sure::ITrampoline {
    public:
        class SuspendContext {
            friend class Coroutine;

        public:
            void Suspend() {
                self_->Suspend();
            }

        private:
            explicit SuspendContext(Coroutine* coro)
                : self_(coro) {}

        private:
            Coroutine* self_;
        };

    public:
        explicit Coroutine(Body coro_body);

        void Resume();
        void Suspend();

        bool IsDone() const;

    private:
        // Allocate 1 Mbyte to callee_stack_;
        static Stack AllocateStack();

        void Run() noexcept override;

        bool is_done_;
        twist::ed::sure::ExecutionContext caller_context_;
        twist::ed::sure::ExecutionContext context_;
        Stack stack_;
        Body coro_body_;
    };

}  // namespace exe::fiber
