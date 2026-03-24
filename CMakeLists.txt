cmake_minimum_required(VERSION 3.20)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(rengine)

find_package(GLEW REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)

add_executable(${PROJECT_NAME}
    sources/main.cpp
    sources/utils.hpp
    sources/stb_image.h
    sources/tiny_obj_loader.h
    sources/tiny_obj_loader.cc

    sources/App.cpp
    sources/App.hpp
    sources/Input.hpp
    sources/Input.cpp
    sources/Buffer.hpp
    sources/Camera.hpp
    sources/Camera.cpp
    sources/Cubemap.hpp
    sources/Cubemap.cpp
    sources/Mesh.hpp
    sources/Mesh.cpp
    sources/Object.hpp
    sources/Object.cpp
    sources/Shader.hpp
    sources/Shader.cpp
    sources/Texture.hpp
    sources/Texture.cpp
    sources/VertexArray.hpp
    sources/VertexArray.cpp
)

target_include_directories(${PROJECT_NAME} PRIVATE sources)

target_link_libraries(${PROJECT_NAME} PRIVATE 
    GLEW::GLEW
    glfw
    glm::glm
)