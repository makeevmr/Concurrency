#pragma once

#include <twist/ed/std/mutex.hpp>

#include <utility>

template <typename T, class Mutex = twist::ed::std::mutex>
class Mutexed {
public:
    class OwnerRef {
    public:
        explicit OwnerRef(T* data_ptr, Mutex& mutex)
            : data_ptr_(data_ptr),
              guard_(mutex) {}

        ~OwnerRef() {}

        const T* operator->() const {
            return data_ptr_;
        }

        T* operator->() {
            return data_ptr_;
        }

        const T& operator*() const& {
            return *data_ptr_;
        }

        T& operator*() & {
            return *data_ptr_;
        }

        T&& operator*() && {
            return *data_ptr_;
        }

    private:
        T* data_ptr_;
        std::lock_guard<Mutex> guard_;
    };

    template <typename... Args>
    explicit Mutexed(Args&&... args)
        : object_(std::forward<Args>(args)...) {}

    OwnerRef Acquire() {
        return OwnerRef(&object_, mutex_);
    }

private:
    T object_;
    Mutex mutex_; // Guards access to object_
};

template <typename T>
auto Acquire(Mutexed<T>& object) {
    return object.Acquire();
}
