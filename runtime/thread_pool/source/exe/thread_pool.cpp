#include <exe/thread_pool.hpp>

#include <twist/ed/static/thread_local/ptr.hpp>

#include <wheels/core/panic.hpp>

TWISTED_STATIC_THREAD_LOCAL_PTR(ThreadPool, pool_ptr);

ThreadPool::ThreadPool(std::size_t threads)
    : is_stopped_(false),
      threads_(threads),
      workers_(),
      task_queue_() {}

ThreadPool::~ThreadPool() {
    assert(is_stopped_);
}

void ThreadPool::Start() {
    for (std::size_t i = 0; i < threads_; ++i) {
        workers_.emplace_back([this]() {
            pool_ptr = this;
            WorkerRoutine();
        });
    }
}

void ThreadPool::Submit(Task new_task) {
    task_queue_.Push(std::move(new_task));
}

ThreadPool* ThreadPool::Current() {
    return pool_ptr;
}

void ThreadPool::Stop() {
    task_queue_.Close();
    for (std::size_t i = 0; i < threads_; ++i) {
        workers_[i].join();
    }
    is_stopped_ = true;
}

void ThreadPool::WorkerRoutine() {
    while (true) {
        std::optional<Task> new_task = task_queue_.Pop();
        if (new_task.has_value()) {
            (*new_task)();
        } else {
            break;
        }
    }
}
