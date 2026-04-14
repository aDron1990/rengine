#include "Input.hpp"
#include "App.hpp"

#include <GLFW/glfw3.h>
#include <cstring>

bool Input::getKey(int key) const noexcept { return m_keys[key]; }
bool Input::getKeyDown(int key) const noexcept { return m_keysDown[key]; }
bool Input::getKeyUp(int key) const noexcept { return m_keysUp[key]; }
bool Input::getButton(int button) const noexcept { return m_buttons[button]; }
bool Input::getButtonDown(int button) const noexcept { return m_buttonsDown[button]; }
bool Input::getButtonUp(int button) const noexcept { return m_buttonsUp[button]; }
glm::ivec2 Input::getCursorPosition() const noexcept { return m_cursorPos; }
std::optional<glm::ivec2> Input::getCursorDelta() const noexcept { return m_cursorDelta; }
std::optional<glm::ivec2> Input::getScrollDelta() const noexcept { return m_scrollDelta; }

Input::~Input()
{
    glfwSetKeyCallback(m_window, nullptr);
    glfwSetMouseButtonCallback(m_window, nullptr);
    glfwSetCursorPosCallback(m_window, nullptr);
    glfwSetScrollCallback(m_window, nullptr);
}

void Input::setGlfwWindow(GLFWwindow* window) noexcept
{
    m_window = window;
    glfwSetKeyCallback(m_window, keyCallback);
    glfwSetMouseButtonCallback(m_window, buttonCallback);
    glfwSetCursorPosCallback(m_window, cursorCallback);
    glfwSetScrollCallback(m_window, scrollCallback);
}

void Input::update() noexcept
{
    std::memset(m_keysDown, 0, sizeof(m_keysDown));
    std::memset(m_keysUp, 0, sizeof(m_keysUp));
    std::memset(m_buttonsDown, 0, sizeof(m_buttonsDown));
    std::memset(m_buttonsUp, 0, sizeof(m_buttonsUp));
    m_cursorDelta.reset();
    m_scrollDelta.reset();
}

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

void Input::buttonCallback(GLFWwindow* window, int button, int action, int mods) noexcept
{
    auto& app = *static_cast<App*>(glfwGetWindowUserPointer(window));
    auto& input = app.getInput();

    if (action == GLFW_PRESS) {
        input.m_buttons[button] = true;
        input.m_buttonsDown[button] = true;
    }
    if (action == GLFW_RELEASE) {
        input.m_buttons[button] = false;
        input.m_buttonsUp[button] = true;
    }
}

void Input::cursorCallback(GLFWwindow* window, double xpos, double ypos) noexcept
{
    auto& app = *static_cast<App*>(glfwGetWindowUserPointer(window));
    auto& input = app.getInput();

    auto newPos = glm::ivec2 { static_cast<int>(xpos), static_cast<int>(ypos) };
    if (input.m_firstMouse) {
        input.m_cursorDelta = { 0, 0 };
        input.m_firstMouse = false;
    } else
        input.m_cursorDelta = input.m_cursorPos - newPos;

    input.m_cursorPos = newPos;
}

void Input::scrollCallback(GLFWwindow* window, double xoffset, double yoffset) noexcept
{
    auto& app = *static_cast<App*>(glfwGetWindowUserPointer(window));
    auto& input = app.getInput();
    input.m_scrollDelta = glm::ivec2 { static_cast<int>(xoffset), static_cast<int>(yoffset) };
}