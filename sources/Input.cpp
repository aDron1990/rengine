#include "Input.hpp"
#include "App.hpp"

void Input::setGlfwWindow(GLFWwindow* window) noexcept {
	glfwSetKeyCallback(window, keyCallback);
}

void Input::update() noexcept {
}

bool Input::getKey(int key) const noexcept {
	return m_keys[key];
}

void Input::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) noexcept {
	auto& app = *static_cast<App*>(glfwGetWindowUserPointer(window));
	auto& input = app.getInput();

	if (action == GLFW_PRESS) input.m_keys[key] = true;
	if (action == GLFW_RELEASE) input.m_keys[key] = false;
}
