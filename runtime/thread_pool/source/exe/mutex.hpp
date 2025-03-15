#pragma once

#include <twist/ed/std/atomic.hpp>
#include <twist/ed/wait/futex.hpp>
#include <twist/ed/wait/spin.hpp>

#include <cstdint>

class Mutex {
public:
    void Lock() {
        MutexStatus free = MutexStatus::FREE;
        if (mutex_status_.compare_exchange_strong(
                free, MutexStatus::LOCKED_NO_CONTENTION)) {
            return;
        }
        twist::ed::SpinWait spin_wait;
        while (mutex_status_.exchange(MutexStatus::LOCKED_WITH_CONTENTION) !=
               MutexStatus::FREE) {
            if (spin_wait.ConsiderParking()) {
                twist::ed::futex::Wait(
                    reinterpret_cast<twist::ed::std::atomic<uint32_t>&>(
                        mutex_status_),
                    static_cast<uint32_t>(MutexStatus::LOCKED_WITH_CONTENTION));
            } else {
                spin_wait();
            }
        }
    }

    void Unlock() {
        auto wake_key = twist::ed::futex::PrepareWake(
            *reinterpret_cast<twist::ed::std::atomic<uint32_t>*>(
                &mutex_status_));
        if (mutex_status_.exchange(MutexStatus::FREE) ==
            MutexStatus::LOCKED_WITH_CONTENTION) {
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
    enum class MutexStatus : uint32_t {
        FREE = 0,
        LOCKED_NO_CONTENTION = 1,
        LOCKED_WITH_CONTENTION = 2
    };

    twist::ed::std::atomic<MutexStatus> mutex_status_{MutexStatus::FREE};
};
