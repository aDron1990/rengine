#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>

class Input {
public:
    Input() = default;
    void setGlfwWindow(GLFWwindow* window) noexcept;

    void update() noexcept;
    bool getKey(int key) const noexcept;
    bool getKeyDown(int key) const noexcept;
    bool getKeyUp(int key) const noexcept;

private:
    static void keyCallback(GLFWwindow* window, int key, int scancode,
        int action, int mods) noexcept;

private:
    bool m_keys[1024] { false };
    bool m_keysDown[1024] { false };
    bool m_keysUp[1024] { false };
};
