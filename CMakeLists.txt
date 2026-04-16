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
    sources/Events.hpp

    sources/utils/utils.hpp
    sources/utils/types.hpp
    sources/utils/glfw.hpp

    sources/App.cpp
    sources/App.hpp
    sources/Input.hpp
    sources/Input.cpp
    sources/systems/RenderSystem.hpp
    sources/systems/RenderSystem.cpp
    sources/systems/PhysicsEngine.hpp
    sources/systems/PhysicsEngine.cpp
    sources/systems/OrbitalEngine.hpp
    sources/systems/OrbitalEngine.cpp
    sources/systems/Clock.hpp
    sources/systems/Clock.cpp

    sources/Model.hpp
    sources/Model.cpp
    sources/Object.hpp
    sources/Object.cpp
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

    sources/graphics/RenderBackend.hpp
    sources/graphics/GlHandle.hpp
    sources/graphics/Buffer.hpp
    sources/graphics/Cubemap.hpp
    sources/graphics/RenderTarget.hpp
    sources/graphics/RenderTexture.hpp
    sources/graphics/RenderTexture.cpp
    sources/graphics/Shader.hpp
    sources/graphics/Shader.cpp
    sources/graphics/Texture.hpp
    sources/graphics/Texture.cpp
    sources/graphics/Mesh.hpp
    sources/graphics/Mesh.cpp
    sources/graphics/VertexArray.hpp
    sources/graphics/VertexArray.cpp

    sources/graphics/opengl/OglRenderBackend.hpp
    sources/graphics/opengl/OglRenderBackend.cpp

    sources/third-party/stb_image.h
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