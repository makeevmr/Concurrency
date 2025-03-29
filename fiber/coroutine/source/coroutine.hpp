#pragma once

#include <sure/context.hpp>
#include <sure/stack/mmap.hpp>

#include <function2/function2.hpp>

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

private:
    using Body = fu2::unique_function<void(SuspendContext)>;

public:
    explicit Coroutine(Body coro_body);

    void Resume();
    void Suspend();

    bool IsDone() const;

private:
    // Allocate 1 Mbyte to callee_stack_;
    static sure::stack::GuardedMmapStack AllocateStack();

    void Run() noexcept override;

    bool is_done_;
    sure::ExecutionContext caller_context_;
    sure::ExecutionContext context_;
    sure::stack::GuardedMmapStack stack_;
    Body coro_body_;
};