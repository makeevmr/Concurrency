#pragma once

#include <twist/ed/std/atomic.hpp>
#include <twist/ed/wait/futex.hpp>
#include <twist/ed/wait/spin.hpp>

#include <cstdint>

class Mutex {
public:
    void Lock() {
        ++waiting_;
        while (state_.exchange(States::Set) == States::Set) {
            twist::ed::futex::Wait(state_, 1);
        }
        --waiting_;
    }

    void Unlock() {
        auto wake_key = twist::ed::futex::PrepareWake(state_);
        state_.store(States::NotSet);
        if (waiting_ != 0) {
            twist::ed::futex::WakeOne(wake_key);
        }
    }

    // BasicLockable
    // https://en.cppreference.com/w/cpp/named_req/BasicLockable

    void lock() { // NOLINT
        Lock();
    }

    void unlock() { // NOLINT
        Unlock();
    }

private:
    struct States {
        enum _ : uint32_t { NotSet = 0, Set = 1 };
    };

    twist::ed::std::atomic<uint32_t> state_{States::NotSet};
    twist::ed::std::atomic<uint32_t> waiting_{0};
};
