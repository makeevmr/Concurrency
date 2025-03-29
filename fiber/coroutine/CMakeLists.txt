begin_task()

set_task_sources(
        coroutine.hpp
        coroutine.cpp)

add_task_playground(play)

add_task_test(unit_tests tests/unit.cpp)

end_task()
