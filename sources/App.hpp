#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <entt/entt.hpp>
#include <string>

#include "Input.hpp"
#include "utils.hpp"

class App {
public:
    App(int windowWidth, int windowHeight, const std::string& windowTitle);
    void run();
    Input& getInput() noexcept { return m_registry.ctx().get<Input>(); };

private:
    void updateWindow() noexcept;
    void processInput(const glm::mat4& viewMatrix, const glm::vec3& cameraPos) noexcept;
    void close() noexcept;

private:
    static void windowCloseCallback(GLFWwindow* window) noexcept;
    static void framebufferSizeCallback(GLFWwindow* window, int width, int height) noexcept;

private:
    App(const App&) = delete;
    App& operator=(const App&) = delete;
    App(App&&) = delete;
    App& operator=(App&&) = delete;

private:
    entt::registry m_registry;
    entt::dispatcher m_dispatcher;

    GlfwContext m_glfwContext;
    GlfwWindowPtr m_window;

    glm::ivec2 m_windowSize;
    bool m_running = true;

    float m_yaw = -90.0f;
    float m_pitch = 0.0f;
};
