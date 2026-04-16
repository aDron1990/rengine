#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/ext/vector_int2.hpp>
#include <glm/glm.hpp>
#include <optional>

class Input {
public:
    Input() = default;
    ~Input();
    void setGlfwWindow(GLFWwindow* window) noexcept;

    void update() noexcept;

    bool getKey(int key) const noexcept;
    bool getKeyDown(int key) const noexcept;
    bool getKeyUp(int key) const noexcept;
    bool getButton(int button) const noexcept;
    bool getButtonDown(int button) const noexcept;
    bool getButtonUp(int button) const noexcept;
    glm::ivec2 getCursorPosition() const noexcept;
    std::optional<glm::ivec2> getCursorDelta() const noexcept;
    std::optional<glm::ivec2> getScrollDelta() const noexcept;

private:
    static void keyCallback(GLFWwindow* window, int key, int scancode,
        int action, int mods) noexcept;
    static void buttonCallback(GLFWwindow* window, int button, int action, int mods) noexcept;
    static void cursorCallback(GLFWwindow* window, double xpos, double ypos) noexcept;
    static void scrollCallback(GLFWwindow* window, double xoffset, double yoffset) noexcept;

private:
    GLFWwindow* m_window { };

    bool m_keys[1024] { false };
    bool m_keysDown[1024] { false };
    bool m_keysUp[1024] { false };

    bool m_buttons[16] { false };
    bool m_buttonsDown[16] { false };
    bool m_buttonsUp[16] { false };

    bool m_firstMouse = true;
    glm::ivec2 m_cursorPos { };
    std::optional<glm::ivec2> m_cursorDelta;
    std::optional<glm::ivec2> m_scrollDelta;
};
