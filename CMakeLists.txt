cmake_minimum_required(VERSION 3.20)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(rengine)

find_package(GLEW REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)

add_executable(${PROJECT_NAME}
    main.cpp
    utils.hpp
    stb_image.h
    tiny_obj_loader.h
    tiny_obj_loader.cc

    App.cpp
    App.hpp
    Input.hpp
    Input.cpp
    Buffer.hpp
    Camera.hpp
    Camera.cpp
    Cubemap.hpp
    Cubemap.cpp
    Mesh.hpp
    Mesh.cpp
    Object.hpp
    Object.cpp
    Shader.hpp
    Shader.cpp
    Texture.hpp
    Texture.cpp
    VertexArray.hpp
    VertexArray.cpp
)

target_link_libraries(${PROJECT_NAME} PRIVATE 
    GLEW::GLEW
    glfw
    glm::glm
)