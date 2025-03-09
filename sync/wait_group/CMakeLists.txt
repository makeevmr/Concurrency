begin_task()

set_task_sources(wait_group.hpp)

# Tests
add_task_test(unit_tests tests/unit.cpp)
add_task_test(random_tests tests/random.cpp)
add_task_test(futex_tests tests/futex.cpp)
add_task_test(model_tests tests/model.cpp)
add_task_test(storage_tests tests/storage.cpp)

end_task()
