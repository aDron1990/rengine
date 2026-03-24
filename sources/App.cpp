#include "App.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <stdexcept>
#include <vector>

#include "Buffer.hpp"
#include "Cubemap.hpp"
#include "Mesh.hpp"
#include "Object.hpp"
#include "Shader.hpp"
#include "Texture.hpp"
#include "VertexArray.hpp"
#include "tiny_obj_loader.h"

App::App(int windowWidth, int windowHeight, const std::string& windowTitle)
    : m_windowSize { windowWidth, windowHeight }
{
    m_window = GlfwWindowPtr(glfwCreateWindow(
        windowWidth, windowHeight, windowTitle.c_str(), nullptr, nullptr));
    if (m_window == nullptr) {
        throw std::runtime_error { "Failed to create GLFW window" };
    }
    m_input.setGlfwWindow(m_window.get());

    glfwSetWindowUserPointer(m_window.get(), this);
    glfwSetCursorPosCallback(m_window.get(), mouseCallback);
    glfwSetInputMode(m_window.get(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    glfwMakeContextCurrent(m_window.get());
    glewExperimental = true;
    if (glewInit() != GLEW_OK) {
        throw std::runtime_error { "Failed to init GLEW" };
    }

    glViewport(0, 0, windowWidth, windowHeight);
    glEnable(GL_DEPTH_TEST);
}

void App::run()
{
    auto cubemapTexture = loadCubemap({ "resources/images/skybox/right.jpg",
        "resources/images/skybox/left.jpg",
        "resources/images/skybox/top.jpg",
        "resources/images/skybox/bottom.jpg",
        "resources/images/skybox/front.jpg",
        "resources/images/skybox/back.jpg" });

    Shader shader { "resources/shaders/main_v.glsl",
        "resources/shaders/main_f.glsl" };
    Shader cubeShader { "resources/shaders/cubemap_v.glsl",
        "resources/shaders/cubemap_f.glsl" };

    auto skyboxVAO = VertexArray { };
    skyboxVAO.bind();
    auto skyboxBuffer = VertexBuffer { skyboxVertices, sizeof(skyboxVertices) };
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float),
        (void*)0);
    glEnableVertexAttribArray(0);

    auto roomMesh = std::make_shared<Mesh>("resources/models/viking_room.obj");
    auto planeMesh = std::make_shared<Mesh>("resources/models/plane.obj");
    auto obMesh = std::make_shared<Mesh>("resources/models/ob.obj");
    auto cubeMesh = std::make_shared<Mesh>("resources/models/cube.obj");

    auto roomTexture = std::make_shared<Texture>("resources/images/viking_room.png");
    auto floorTexture = std::make_shared<Texture>("resources/images/floor.jpg");
    auto grungeTexture = std::make_shared<Texture>("resources/images/grunge.jpg");
    auto rustTexture = std::make_shared<Texture>("resources/images/rust.jpg");

    Object room { m_registry, roomMesh, roomTexture };
    Object floor { m_registry, planeMesh, floorTexture };
    Object ob { m_registry, obMesh, grungeTexture };
    Object cube { m_registry, cubeMesh, rustTexture };

    floor.scale() *= 2.5f;
    floor.position() += glm::vec3 { 0.0f, -0.1f, 0.0f };

    room.rotation() = { -90.0f, 0.0f, 180.0f };
    room.position() = { 0.0f, 0.0f, -1.3f };

    ob.position() = { 0.0f, 0.3f, 0.0f };
    ob.rotation() = { 0.0f, -90.0f, 0.0f };

    cube.position() = { 0.0f, 0.3f, 1.3f };

    while (m_running) {
        updateWindow();
        processInput();
        render();

        static float fov = 60.0f;
        if (m_input.getKey(GLFW_KEY_UP))
            fov -= 0.3f;
        if (m_input.getKey(GLFW_KEY_DOWN))
            fov += 0.3f;

        auto proj = glm::perspective(glm::radians(fov),
            (float)m_windowSize.x / m_windowSize.y,
            0.1f, 100.0f);
        auto& view = m_camera.getView();

        {
            auto view_ = glm::mat4(glm::mat3(view));

            glDepthMask(GL_FALSE);
            cubeShader.use();
            cubeShader.setUniform(view_, "view");
            cubeShader.setUniform(proj, "proj");
            skyboxVAO.bind();
            glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
            glDrawArrays(GL_TRIANGLES, 0, 36);
            glDepthMask(GL_TRUE);
        }

        shader.use();
        shader.setUniform(view, "view");
        shader.setUniform(proj, "proj");

        room.draw(shader);
        floor.draw(shader);
        ob.draw(shader);
        cube.draw(shader);

        glfwSwapBuffers(m_window.get());
    }
}

Input& App::getInput() noexcept { return m_input; }

void App::updateWindow() noexcept { glfwPollEvents(); }

void App::processInput() noexcept
{
    if (m_input.getKey(GLFW_KEY_W))
        m_camera.move(Camera::Direction::Front);
    if (m_input.getKey(GLFW_KEY_S))
        m_camera.move(Camera::Direction::Back);
    if (m_input.getKey(GLFW_KEY_A))
        m_camera.move(Camera::Direction::Left);
    if (m_input.getKey(GLFW_KEY_D))
        m_camera.move(Camera::Direction::Right);
    if (m_input.getKey(GLFW_KEY_SPACE))
        m_camera.move(Camera::Direction::Up);
    if (m_input.getKey(GLFW_KEY_LEFT_SHIFT))
        m_camera.move(Camera::Direction::Down);

    m_camera.update();

    if (m_input.getKey(GLFW_KEY_ESCAPE))
        close();
}

void App::render() noexcept
{
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void App::close() noexcept { m_running = false; }

void App::mouseCallback(GLFWwindow* window, double xpos, double ypos) noexcept
{
    auto* app = static_cast<App*>(glfwGetWindowUserPointer(window));
    if (app->m_firstMouse) {
        app->m_lastX = xpos;
        app->m_lastY = ypos;
        app->m_firstMouse = false;
    }

    float xoffset = xpos - app->m_lastX;
    float yoffset = app->m_lastY - ypos;
    app->m_lastX = xpos;
    app->m_lastY = ypos;

    float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    app->m_yaw += xoffset;
    app->m_pitch += yoffset;

    app->m_camera.rotate(app->m_yaw, app->m_pitch);
}
