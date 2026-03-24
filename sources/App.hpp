#pragma once

#include <entt/entt.hpp>
#include <string>

#include "Camera.hpp"
#include "Input.hpp"
#include "utils.hpp"

class App {
public:
    App(int windowWidth, int windowHeight, const std::string& windowTitle);
    void run();
    Input& getInput() noexcept;

private:
    void updateWindow() noexcept;
    void processInput() noexcept;
    void close() noexcept;

private:
    App(const App&) = delete;
    App& operator=(const App&) = delete;
    App(App&&) = delete;
    App& operator=(App&&) = delete;

private:
    static void mouseCallback(GLFWwindow* window, double xpos,
        double ypos) noexcept;

private:
    GlfwContext m_glfwContext;
    GlfwWindowPtr m_window;
    Input m_input;
    Camera m_camera { glm::vec3 { 0.0f, 0.0f, 3.0f } };
    entt::registry m_registry;

    glm::ivec2 m_windowSize;
    bool m_running = true;

    bool m_firstMouse = true;
    double m_lastX { };
    double m_lastY { };

    float m_yaw = -90.0f;
    float m_pitch = 0.0f;
};
