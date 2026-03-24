#pragma once

#include "utils.hpp"

class Input {
public:
	Input() = default;
	void setGlfwWindow(GLFWwindow* window) noexcept;

	void update() noexcept;
	bool getKey(int key) const noexcept;

private:
	static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) noexcept;

private:
	bool m_keys[1024]{ false };

};

