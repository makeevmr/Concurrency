#pragma once

#include <twist/ed/std/atomic.hpp>
#include <twist/ed/wait/futex.hpp>

#include <cstdint>

class CondVar {
public:
    // Mutex - BasicLockable
    // https://en.cppreference.com/w/cpp/named_req/BasicLockable
    template <class Mutex>
    void Wait(Mutex& mutex) {
        uint32_t prev_cv_state = current_cv_state_.load();
        mutex.unlock();
        twist::ed::futex::Wait(current_cv_state_, prev_cv_state);
        mutex.lock();
    }

    void NotifyOne() {
        auto wake_key = twist::ed::futex::PrepareWake(current_cv_state_);
        current_cv_state_.fetch_add(1);
        twist::ed::futex::WakeOne(wake_key);
    }

    void NotifyAll() {
        auto wake_key = twist::ed::futex::PrepareWake(current_cv_state_);
        current_cv_state_.fetch_add(1);
        twist::ed::futex::WakeAll(wake_key);
    }

private:
    twist::ed::std::atomic<uint32_t> current_cv_state_{0};
};
