#pragma once

template <typename Mutex>
class LockGuard {
public:
    explicit LockGuard(Mutex& mutex)
        : mutex_owner_(mutex) {
        mutex_owner_.lock();
    }

    ~LockGuard() {
        mutex_owner_.unlock();
    }

    // Non-copyable
    LockGuard(const LockGuard&) = delete;
    LockGuard& operator=(const LockGuard&) = delete;

    // Non-movable
    LockGuard(LockGuard&&) = delete;
    LockGuard& operator=(LockGuard&&) = delete;

private:
    Mutex& mutex_owner_;
};