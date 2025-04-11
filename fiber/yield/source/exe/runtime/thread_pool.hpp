#pragma once

#include "task.hpp"
#include "queue.hpp"

#include <twist/ed/std/thread.hpp>

namespace exe::runtime {

    class ThreadPool {
    public:
        explicit ThreadPool(std::size_t threads);
        ~ThreadPool();

        // Non-copyable
        ThreadPool(const ThreadPool&) = delete;
        ThreadPool& operator=(const ThreadPool&) = delete;

        // Non-movable
        ThreadPool(ThreadPool&&) = delete;
        ThreadPool& operator=(ThreadPool&&) = delete;

        void Start();

        void Submit(Task);

        static ThreadPool* Current();

        void Stop();

    private:
        void WorkerRoutine();

        bool is_stopped_;
        std::size_t threads_;
        std::vector<twist::ed::std::thread> workers_;
        UnboundedBlockingQueue<Task> task_queue_;
    };

}  // namespace exe::runtime
