#pragma once

#include <twist/ed/std/mutex.hpp>
#include <twist/ed/std/condition_variable.hpp>

#include <cstddef>

class CyclicBarrier {
public:
    explicit CyclicBarrier(size_t participants)
        : participants_(participants),
          woken_up_(0),
          to_arrive_(participants),
          mutex_(),
          cond_var_() {}

    void ArriveAndWait() {
        std::unique_lock<twist::ed::std::mutex> u_lock(mutex_);
        cond_var_.wait(u_lock, [this]() {
            return participants_ != 0;
        });
        --participants_;
        cond_var_.wait(u_lock, [this]() {
            return participants_ == 0;
        });
        if (participants_ == 0) {
            cond_var_.notify_all();
        }
        ++woken_up_;
        if (woken_up_ == to_arrive_) {
            woken_up_ = 0;
            participants_ = to_arrive_;
            cond_var_.notify_all();
        }
    }

private:
    std::size_t participants_;
    std::size_t woken_up_;
    const std::size_t to_arrive_;
    twist::ed::std::mutex mutex_;
    twist::ed::std::condition_variable cond_var_;
};
