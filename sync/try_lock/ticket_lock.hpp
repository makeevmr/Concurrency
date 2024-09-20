#pragma once

#include <twist/ed/std/atomic.hpp>
#include <twist/ed/wait/spin.hpp>

#include <cstdint>

class TicketLock {
    using Ticket = uint64_t;

public:
    // Do not change this method
    void Lock() {
        const Ticket this_thread_ticket = next_free_ticket_.fetch_add(1);

        twist::ed::SpinWait spin_wait;
        while (this_thread_ticket != owner_ticket_.load()) {
            spin_wait();
        }
    }

    bool TryLock() {
        Ticket expected = owner_ticket_.load();
        return next_free_ticket_.compare_exchange_strong(expected, next_free_ticket_.load() + 1);
    }

    // Do not change this method
    void Unlock() {
        // Do we actually need atomic increment here?
        owner_ticket_.fetch_add(1);
    }

private:
    twist::ed::std::atomic<Ticket> next_free_ticket_{0};
    twist::ed::std::atomic<Ticket> owner_ticket_{0};
};
