#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <entt/entt.hpp>
#include <string>

#include "Input.hpp"
#include "Scene.hpp"
#include "utils/glfw.hpp"

class App {
public:
    App(int windowWidth, int windowHeight, const std::string& windowTitle);
    void run();
    Input& getInput() noexcept { return registry().ctx().get<Input>(); };

private:
    entt::registry& registry() noexcept { return m_scene.registry(); }
    const entt::registry& registry() const noexcept { return m_scene.registry(); }

    void updateWindow() noexcept;
    void processInput(entt::entity cameraEntity, const glm::mat4& viewMatrix, const glm::vec3& cameraPos) noexcept;
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
    Scene m_scene;
    entt::dispatcher m_dispatcher;

    GlfwContext m_glfwContext;
    GlfwWindowPtr m_window;

    glm::ivec2 m_windowSize;
    bool m_running = true;

    float m_yaw = -90.0f;
    float m_pitch = 0.0f;
};
