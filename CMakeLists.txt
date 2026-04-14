cmake_minimum_required(VERSION 3.20)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(rengine)

find_package(GLEW REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(EnTT CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(Jolt CONFIG REQUIRED)
find_package(assimp CONFIG REQUIRED)

add_executable(${PROJECT_NAME}
    sources/main.cpp
    sources/utils.hpp
    sources/Events.hpp

    sources/App.cpp
    sources/App.hpp
    sources/Input.hpp
    sources/Input.cpp
    sources/Buffer.hpp

    sources/Cubemap.hpp
    sources/RenderTarget.hpp
    sources/RenderTexture.hpp
    sources/RenderTexture.cpp

    sources/Mesh.hpp
    sources/Mesh.cpp
    sources/Model.hpp
    sources/Model.cpp
    sources/Object.hpp
    sources/Object.cpp
    sources/Shader.hpp
    sources/Shader.cpp
    sources/Texture.hpp
    sources/Texture.cpp
    sources/VertexArray.hpp
    sources/VertexArray.cpp
    sources/LineBatch.hpp
    sources/LineBatch.cpp
    sources/BoundingBox.hpp

    sources/objects/FlyingCamera.hpp
    sources/objects/FlyingCamera.cpp
    sources/objects/OrbitCamera.hpp
    sources/objects/OrbitCamera.cpp
    sources/objects/ModelObject.hpp
    sources/objects/ModelObject.cpp
    sources/objects/TestSatelite.cpp
    sources/objects/TestSatelite.hpp

    sources/components/Transform.hpp
    sources/components/Transform.cpp
    sources/components/Camera.hpp
    sources/components/Camera.cpp
    sources/components/Renderer.hpp
    sources/components/Material.hpp
    sources/components/Body.hpp
    sources/components/Celestial.hpp
    sources/components/OrbitalBody.hpp

    sources/systems/RendererSystem.hpp
    sources/systems/RendererSystem.cpp
    sources/systems/PhysicsEngine.hpp
    sources/systems/PhysicsEngine.cpp
    sources/systems/OrbitalEngine.hpp
    sources/systems/OrbitalEngine.cpp
    sources/systems/Clock.hpp
    sources/systems/Clock.cpp

    sources/third-party/stb_image.h
    sources/third-party/tiny_obj_loader.h
    sources/third-party/tiny_obj_loader.cc
)

target_include_directories(${PROJECT_NAME} PRIVATE sources)

target_link_libraries(${PROJECT_NAME} PRIVATE
    GLEW::GLEW
    glfw
    glm::glm
    EnTT::EnTT
    imgui::imgui
    Jolt::Jolt
    assimp::assimp
)