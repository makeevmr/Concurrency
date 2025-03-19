#pragma once

#include "error.hpp"
#include <twist/ed/std/atomic.hpp>
#include <twist/ed/std/condition_variable.hpp>
#include <twist/ed/std/mutex.hpp>

#include <memory>
#include <expected>
#include <mutex>

template <typename T>
class Future {
    template <typename U>
    friend class Promise;

public:
    // Non-copyable
    Future(const Future&) = delete;
    Future& operator=(const Future&) = delete;

    // Movable
    Future(Future&&) = default;
    Future& operator=(Future&&) = default;

    // One-shot
    T Get() {
        const twist::ed::std::atomic<State>& curr_state =
            future_handler_ptr_->state_;
        if (curr_state == State::CONSUMED) {
            throw NoStateError();
        }
        std::unique_lock<twist::ed::std::mutex> u_lock(
            future_handler_ptr_->mutex_);
        future_handler_ptr_->is_produced_.wait(u_lock, [this]() {
            return future_handler_ptr_->state_ == State::PRODUCED;
        });
        future_handler_ptr_->state_ = State::CONSUMED;
        if (future_handler_ptr_->expected_value_.has_value()) {
            return std::move(future_handler_ptr_->expected_value_.value());
        }
        std::rethrow_exception(future_handler_ptr_->expected_value_.error());
    }

    bool Valid() const {
        const twist::ed::std::atomic<State>& curr_state =
            future_handler_ptr_->state_;
        return (curr_state == State::SHARED || curr_state == State::PRODUCED);
    }

private:
    enum class State : int { INITIALIZED = 0, SHARED, PRODUCED, CONSUMED };

    struct FutureHandler {
        twist::ed::std::atomic<State> state_{State::INITIALIZED};
        twist::ed::std::mutex mutex_;
        twist::ed::std::condition_variable is_produced_;
        std::expected<T, std::exception_ptr> expected_value_;
    };

    explicit Future(const std::shared_ptr<FutureHandler>& future_value)
        : future_handler_ptr_(future_value) {}

private:
    std::shared_ptr<FutureHandler> future_handler_ptr_;
};
