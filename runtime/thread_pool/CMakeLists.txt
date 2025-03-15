begin_task()

add_task_library(exe)

# WaitGroup
add_task_test(wg_unit_tests tests/wait_group/unit.cpp)
add_task_test(wg_random_tests tests/wait_group/random.cpp)
add_task_test(wg_futex_tests tests/wait_group/futex.cpp)
add_task_test(wg_model_tests tests/wait_group/model.cpp)
add_task_test(wg_storage_tests tests/wait_group/storage.cpp)

# UnboundedBlockingQueue
add_task_test(queue_unit_tests tests/queue/unit.cpp)
add_task_test(queue_random_tests tests/queue/random.cpp)
add_task_test(queue_stress_tests tests/queue/stress.cpp)

# ThreadPool
add_task_test(tp_unit_tests tests/thread_pool/unit.cpp)
add_task_test(tp_random_tests tests/thread_pool/random.cpp)
add_task_test(tp_stress_tests tests/thread_pool/stress.cpp)

add_task_playground(play)

end_task()
