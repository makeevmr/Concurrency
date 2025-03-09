#pragma once

#include <cstddef>
#include <twist/ed/std/atomic.hpp>
#include <twist/ed/std/mutex.hpp>
#include <twist/ed/std/condition_variable.hpp>

class WaitGroup {
public:
    void Add(size_t count) {
        count_.fetch_add(count);
    }

    void Done() {
        std::lock_guard<twist::ed::std::mutex> lock(mutex_);
        count_.fetch_add(-1);
        if (count_.load() == 0 && waiters_.load() > 0) {
            cond_var_.notify_all();
        }
    }

    void Wait() {
        std::unique_lock<twist::ed::std::mutex> u_lock(mutex_);
        if (count_.load() > 0) {
            ++waiters_;
            cond_var_.wait(u_lock, [this]() {
                return count_.load() == 0;
            });
            --waiters_;
        }
    }

private:
    twist::ed::std::atomic<uint64_t> count_{0};
    twist::ed::std::atomic<uint64_t> waiters_{0};
    twist::ed::std::mutex mutex_;
    twist::ed::std::condition_variable cond_var_;
};
