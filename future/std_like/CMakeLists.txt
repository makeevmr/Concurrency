begin_task()

add_task_library(exe)

# Tests
add_task_test(unit_tests tests/unit.cpp)
add_task_test(confuse_tests tests/confuse.cpp)
add_task_test(model_tests tests/model.cpp)
add_task_test(random_tests tests/random.cpp)

add_task_playground(play)

end_task()
