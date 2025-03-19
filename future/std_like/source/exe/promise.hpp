#pragma once

#include "future.hpp"
#include <type_traits>

template <typename T>
class Promise {
public:
    Promise()
        : future_handler_ptr_(ConstructPromise()) {}

    // Non-copyable
    Promise(const Promise&) = delete;
    Promise& operator=(const Promise&) = delete;

    // Movable
    Promise(Promise&&) = default;
    Promise& operator=(Promise&&) = default;

    ~Promise() {
        if (future_handler_ptr_ != nullptr) {
            std::lock_guard<twist::ed::std::mutex> guard(
                future_handler_ptr_->mutex_);
            twist::ed::std::atomic<State>& curr_state =
                future_handler_ptr_->state_;
            if (curr_state == State::SHARED) {
                future_handler_ptr_->expected_value_ =
                    std::unexpected<std::exception_ptr>(
                        std::make_exception_ptr<BrokenPromiseError>(
                            BrokenPromiseError()));
                curr_state = State::PRODUCED;
                future_handler_ptr_->is_produced_.notify_one();
            }
        }
    }

    // One-shot
    Future<T> MakeFuture() {
        if (future_handler_ptr_->state_ == State::INITIALIZED) {
            future_handler_ptr_->state_ = State::SHARED;
        }
        return Future<T>(future_handler_ptr_);
    }

    // One-shot
    void SetValue(T value) {
        if (future_handler_ptr_ == nullptr) {
            throw NoStateError();
        }
        std::lock_guard<twist::ed::std::mutex> guard(
            future_handler_ptr_->mutex_);
        SetProducedState();
        future_handler_ptr_->expected_value_ = std::move(value);
    }

    // One-shot
    void SetException(std::exception_ptr exception_ptr) {
        if (future_handler_ptr_ == nullptr) {
            throw NoStateError();
        }
        std::lock_guard<twist::ed::std::mutex> guard(
            future_handler_ptr_->mutex_);
        SetProducedState();
        future_handler_ptr_->expected_value_ =
            std::unexpected<std::exception_ptr>(exception_ptr);
    }

private:
    constexpr std::shared_ptr<typename Future<T>::FutureHandler>
    ConstructPromise() {
        if constexpr (std::is_default_constructible_v<T>) {
            return std::make_shared<typename Future<T>::FutureHandler>(
                State::INITIALIZED);
        }
        return nullptr;
    }

    void SetProducedState() {
        twist::ed::std::atomic<State>& curr_state = future_handler_ptr_->state_;
        if (curr_state == State::PRODUCED || curr_state == State::CONSUMED) {
            throw PromiseAlreadySatisfiedError();
        }
        curr_state = State::PRODUCED;
        future_handler_ptr_->is_produced_.notify_one();
    }

    using State = Future<T>::State;

    std::shared_ptr<typename Future<T>::FutureHandler> future_handler_ptr_;
};
