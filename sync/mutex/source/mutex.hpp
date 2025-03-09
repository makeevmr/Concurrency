#pragma once

#include <twist/ed/std/atomic.hpp>
#include <twist/ed/wait/futex.hpp>
#include <twist/ed/wait/spin.hpp>

#include <cstdint>

class Mutex {
public:
    void Lock() {
        ++waiters_;
        twist::ed::SpinWait spin_wait;
        while (is_locked_.exchange(1) == 1) {
            if (spin_wait.ConsiderParking()) {
                twist::ed::futex::Wait(is_locked_, 1);
            } else {
                spin_wait();
            }
        }
        --waiters_;
    }

    void Unlock() {
        auto wake_key = twist::ed::futex::PrepareWake(is_locked_);
        is_locked_.store(0);
        if (waiters_ > 0) {
            twist::ed::futex::WakeOne(wake_key);
        }
    }

    // BasicLockable
    // https://en.cppreference.com/w/cpp/named_req/BasicLockable

    void lock() {  // NOLINT
        Lock();
    }

    void unlock() {  // NOLINT
        Unlock();
    }

private:
    twist::ed::std::atomic<uint32_t> is_locked_ = 0;
    twist::ed::std::atomic<uint32_t> waiters_ = 0;
};
