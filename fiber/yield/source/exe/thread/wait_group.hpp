#pragma once

#include "../fiber/sync/condvar.hpp"
#include "../fiber/sync/mutex.hpp"

#include <mutex>
#include <twist/ed/std/atomic.hpp>
#include <cstddef>

namespace exe::thread {

    class WaitGroup {
    public:
        void Add(size_t count) {
            count_.fetch_add(count);
        }

        void Done() {
            std::lock_guard<Mutex> lock(mutex_);
            count_.fetch_add(-1);
            if (count_.load() == 0 && waiters_.load() > 0) {
                is_positive_count_.NotifyAll();
            }
        }

        void Wait() {
            std::unique_lock<Mutex> u_lock(mutex_);
            if (count_.load() > 0) {
                ++waiters_;
                while (count_.load() != 0) {
                    is_positive_count_.Wait(u_lock);
                }
                --waiters_;
            }
        }

    private:
        twist::ed::std::atomic<uint64_t> count_{0};
        twist::ed::std::atomic<uint64_t> waiters_{0};
        Mutex mutex_;
        CondVar is_positive_count_;
    };

}  // namespace exe::thread
