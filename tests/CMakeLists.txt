add_executable(rengine_tests
    TestMain.cpp
    ../sources/Object.cpp
    ../sources/Scene.cpp
    ../sources/systems/OriginRebaseSystem.cpp
    ../sources/components/Transform.cpp
)

target_include_directories(rengine_tests PRIVATE
    ../sources
)

target_link_libraries(rengine_tests PRIVATE
    doctest::doctest
    EnTT::EnTT
    glm::glm
)

add_test(NAME rengine_tests COMMAND rengine_tests)
