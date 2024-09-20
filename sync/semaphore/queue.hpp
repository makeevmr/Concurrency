#pragma once

#include "tagged_semaphore.hpp"

#include <deque>

// Bounded Blocking Multi-Producer/Multi-Consumer (MPMC) Queue

template <typename T>
class BoundedBlockingQueue {
public:
    explicit BoundedBlockingQueue(size_t capacity)
        : lock_(1),
          empty_cells_sem_(capacity),
          filled_cells_sem_(0),
          deque_() {}

    void Put(T new_elem) {
        typename TaggedSemaphore<SemTag>::Token empty_token = empty_cells_sem_.Acquire();
        typename TaggedSemaphore<SemTag>::Token token = lock_.Acquire();
        deque_.push_back(std::move(new_elem));
        lock_.Release(std::move(token));
        filled_cells_sem_.Release(std::move(empty_token));
    }

    T Take() {
        typename TaggedSemaphore<SemTag>::Token filled_token = filled_cells_sem_.Acquire();
        typename TaggedSemaphore<SemTag>::Token token = lock_.Acquire();
        T front_elem = std::move(deque_.front());
        deque_.pop_front();
        lock_.Release(std::move(token));
        empty_cells_sem_.Release(std::move(filled_token));
        return front_elem;
    }

private:
    // Tags
    struct SemTag {};

private:
    TaggedSemaphore<SemTag> lock_;
    TaggedSemaphore<SemTag> empty_cells_sem_;
    TaggedSemaphore<SemTag> filled_cells_sem_;
    std::deque<T> deque_;
};