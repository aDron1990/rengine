#include "Input.hpp"
#include "App.hpp"

#include <cstring>

void Input::setGlfwWindow(GLFWwindow* window) noexcept
{
    glfwSetKeyCallback(window, keyCallback);
}

void Input::update() noexcept
{
    std::memset(m_keysDown, 0, sizeof(m_keysDown));
    std::memset(m_keysUp, 0, sizeof(m_keysUp));
}

bool Input::getKey(int key) const noexcept { return m_keys[key]; }
bool Input::getKeyDown(int key) const noexcept { return m_keysDown[key]; }
bool Input::getKeyUp(int key) const noexcept { return m_keysUp[key]; }

void Input::keyCallback(GLFWwindow* window, int key, int scancode, int action,
    int mods) noexcept
{
    auto& app = *static_cast<App*>(glfwGetWindowUserPointer(window));
    auto& input = app.getInput();

    if (action == GLFW_PRESS) {
        input.m_keys[key] = true;
        input.m_keysDown[key] = true;
    }
    if (action == GLFW_RELEASE) {
        input.m_keys[key] = false;
        input.m_keysUp[key] = true;
    }
}
