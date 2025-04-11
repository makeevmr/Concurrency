#pragma once

#include "../fiber/sync/condvar.hpp"
#include "../fiber/sync/mutex.hpp"

#include <mutex>

#include <deque>
#include <optional>

// Unbounded blocking multi-producers/multi-consumers (MPMC) queue

template <typename T>
class UnboundedBlockingQueue {
public:
    void Push(T new_value) {
        std::lock_guard<Mutex> guard(mutex_);
        queue_buffer_.push_back(std::move(new_value));
        if (waiters_ > 0) {
            not_empty_buffer_.NotifyOne();
        }
    }

    std::optional<T> Pop() {
        std::unique_lock<Mutex> u_lock(mutex_);
        if (queue_buffer_.empty() && is_queue_closed_) {
            return std::nullopt;
        }
        ++waiters_;
        while (queue_buffer_.empty() && !is_queue_closed_) {
            not_empty_buffer_.Wait(u_lock);
        }
        --waiters_;
        if (queue_buffer_.empty()) {
            return std::nullopt;
        }
        std::optional<T> front_element{std::move(queue_buffer_.front())};
        queue_buffer_.pop_front();
        return front_element;
    }

    void Close() {
        std::lock_guard<Mutex> guard(mutex_);
        is_queue_closed_ = true;
        if (waiters_ > 0) {
            not_empty_buffer_.NotifyAll();
        }
    }

private:
    bool is_queue_closed_{false};
    uint32_t waiters_{0};
    Mutex mutex_;
    CondVar not_empty_buffer_;
    std::deque<T> queue_buffer_;  // Guarded by mutex_
};
