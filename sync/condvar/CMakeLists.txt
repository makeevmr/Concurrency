begin_task()
set_task_sources(condvar.hpp mutex.hpp)

# Condition Variable
add_task_test(condvar_unit_tests tests/condvar/unit.cpp)
add_task_test(condvar_random_tests tests/condvar/random.cpp)
add_task_test(condvar_stress_tests tests/condvar/stress.cpp)
add_task_test(condvar_model_tests tests/condvar/model.cpp)

# Mutex
add_task_test(mutex_unit_tests tests/mutex/unit.cpp)
add_task_test(mutex_stress_tests tests/mutex/stress.cpp)
add_task_test(mutex_model_tests tests/mutex/model.cpp)
add_task_test(mutex_random_tests tests/mutex/random.cpp)
add_task_test(mutex_futex_tests tests/mutex/futex.cpp)

# Event
add_task_test(event_storage_tests tests/event/storage.cpp)

end_task()
