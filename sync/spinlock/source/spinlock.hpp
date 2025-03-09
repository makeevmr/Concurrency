#pragma once

#include "atomic.hpp"
#include "hint.hpp"

// Naive Test-and-Set (TAS) spinlock

class TASSpinLock {
public:
    void Lock() {
        while (locked_.Exchange(1) == 1) {
            SpinLoopHint();
        }
    }

    bool TryLock() {
        return locked_.Exchange(1) == 0;
    }

    void Unlock() {
        locked_.Store(0);
    }

private:
    Atomic locked_{0};
};
