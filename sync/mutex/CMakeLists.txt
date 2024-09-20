begin_task()

set_task_sources(mutex.hpp)

add_task_test(unit_tests tests/unit.cpp)
add_task_test(random_tests tests/random.cpp)
add_task_test(stress_tests tests/stress.cpp)
add_task_test(model_tests tests/model.cpp)
add_task_test(futex_tests tests/futex.cpp)

end_task()
