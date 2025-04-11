begin_task()

add_task_library(exe)

add_task_playground(play)

# Tests

# WaitGroup
add_task_test(wg_unit_tests tests/thread/wait_group/unit.cpp)
add_task_test(wg_random_tests tests/thread/wait_group/random.cpp)
add_task_test(wg_futex_tests tests/thread/wait_group/futex.cpp)
add_task_test(wg_model_tests tests/thread/wait_group/model.cpp)
add_task_test(wg_storage_tests tests/thread/wait_group/storage.cpp)

# ThreadPool
add_task_test(runtime_tp_unit_tests tests/runtime/thread_pool/unit.cpp)
add_task_test(runtime_tp_random_tests tests/runtime/thread_pool/random.cpp)
add_task_test(runtime_tp_stress_tests tests/runtime/thread_pool/stress.cpp)

# Fibers
add_task_test(fiber_sched_tp_unit_tests tests/fiber/sched/thread_pool/unit.cpp)
add_task_test(fiber_sched_tp_stress_tests tests/fiber/sched/thread_pool/stress.cpp)

end_task()
