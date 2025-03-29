#include "coroutine.hpp"

Coroutine::Coroutine(Body coro_body)
    : is_done_(false),
      caller_context_(),
      context_(),
      stack_(AllocateStack()),
      coro_body_(std::move(coro_body)) {
    context_.Setup(stack_.MutView(), this);
}

void Coroutine::Resume() {
    caller_context_.SwitchTo(context_);
}

void Coroutine::Suspend() {
    context_.SwitchTo(caller_context_);
}

bool Coroutine::IsDone() const {
    return is_done_;
}

void Coroutine::Run() noexcept {
    coro_body_(SuspendContext{this});
    is_done_ = true;
    context_.ExitTo(caller_context_);
}

// Allocate 1 MB to stack_;
sure::stack::GuardedMmapStack Coroutine::AllocateStack() {
    static constexpr std::size_t kSize = 1024 * 1024;
    return sure::stack::GuardedMmapStack::AllocateAtLeastBytes(kSize);
}