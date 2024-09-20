#pragma once

#include <twist/ed/std/mutex.hpp>
#include <twist/ed/std/condition_variable.hpp>

#include <cstdlib>

class Semaphore {
public:
    explicit Semaphore(size_t tokens)
        : tokens_(tokens),
          cond_var_(),
          mutex_() {}

    void Acquire() {
        std::unique_lock<twist::ed::std::mutex> u_lock(mutex_);
        cond_var_.wait(u_lock, [this] {
            return tokens_ > 0;
        });
        --tokens_;
        u_lock.unlock();
    }

    void Release() {
        std::lock_guard<twist::ed::std::mutex> g_lock(mutex_);
        ++tokens_;
        cond_var_.notify_one();
    }

private:
    std::size_t tokens_;
    twist::ed::std::condition_variable cond_var_;
    twist::ed::std::mutex mutex_;
};