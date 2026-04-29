add_executable(rengine_tests
    TestMain.cpp
)

target_link_libraries(rengine_tests PRIVATE
    doctest::doctest
)

add_test(NAME rengine_tests COMMAND rengine_tests)
