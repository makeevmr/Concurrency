begin_task()

set_task_sources(
        semaphore.hpp
        tagged_semaphore.hpp
        queue.hpp)

# Semaphore
add_task_test(sema_unit_tests tests/semaphore/unit.cpp)
add_task_test(sema_random_tests tests/semaphore/random.cpp)
add_task_test(sema_stress_tests tests/semaphore/stress.cpp)

# Queue
add_task_test(queue_unit_tests tests/queue/unit.cpp)
add_task_test(queue_random_tests tests/queue/random.cpp)
add_task_test(queue_stress_tests tests/queue/stress.cpp)

end_task()
