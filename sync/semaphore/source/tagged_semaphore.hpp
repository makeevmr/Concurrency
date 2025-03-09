#pragma once

#include "semaphore.hpp"

#include <cassert>

template <class Tag>
class TaggedSemaphore {
public:
    // ~ Linear
    class Permit {
        friend class TaggedSemaphore;

    public:
        ~Permit() {
            assert(!valid_);
        }

        // Non-copyable
        Permit(const Permit&) = delete;
        Permit& operator=(const Permit&) = delete;

        // Movable
        Permit(Permit&& that) {
            that.Invalidate();
        }

        Permit& operator=(Permit&&) = delete;

    private:
        Permit() = default;

        void Invalidate() {
            assert(valid_);
            valid_ = false;
        }

    private:
        bool valid_{true};
    };

public:
    explicit TaggedSemaphore(size_t permits)
        : impl_(permits) {}

    Permit Acquire() {
        impl_.Acquire();
        return Permit{};
    }

    void Release(Permit&& permit) {
        impl_.Release();
        permit.Invalidate();
    }

private:
    Semaphore impl_;
};
