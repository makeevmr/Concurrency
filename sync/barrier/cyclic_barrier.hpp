#pragma once

#include <twist/ed/std/mutex.hpp>
#include <twist/ed/std/condition_variable.hpp>

#include <cstddef>

class CyclicBarrier {
public:
    explicit CyclicBarrier(size_t participants)
        : queue_threads_(0),
          to_pass_threads_(0),
          group_size_(participants),
          mutex_(),
          cond_var_() {}

    void ArriveAndWait() {
        std::unique_lock<twist::ed::std::mutex> u_lock(mutex_);
        cond_var_.wait(u_lock, [this] {
            return to_pass_threads_ == 0;
        });
        ++queue_threads_;
        if (queue_threads_ == group_size_ && to_pass_threads_ == 0) {
            to_pass_threads_ = group_size_;
        }
        cond_var_.wait(u_lock, [this] {
            return to_pass_threads_ > 0;
        });
        --queue_threads_;
        --to_pass_threads_;
        if ((to_pass_threads_ == group_size_ - 1) || (to_pass_threads_ == 0)) {
            u_lock.unlock();
            cond_var_.notify_all();
        } else {
            u_lock.unlock();
        }
    }

private:
    std::size_t queue_threads_;
    std::size_t to_pass_threads_;
    std::size_t group_size_;
    twist::ed::std::mutex mutex_;
    twist::ed::std::condition_variable cond_var_;
};
