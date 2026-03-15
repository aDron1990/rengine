#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <stdexcept>
#include <memory>

struct GlfwContext {
	GlfwContext() {
		if (!glfwInit()) {
			throw std::runtime_error{ "Failed to init GLFW" };
		}
	}

	~GlfwContext() {
		glfwTerminate();
	}

	GlfwContext(const GlfwContext&) = delete;
	GlfwContext& operator=(const GlfwContext&) = delete;
	GlfwContext(GlfwContext&&) = delete;
	GlfwContext& operator=(GlfwContext&&) = delete;
};

struct GlfwWindowDeleter {
	void operator()(GLFWwindow* window) const {
		if (window != nullptr)
			glfwDestroyWindow(window);
	}
};

using GlfwWindowPtr = std::unique_ptr<GLFWwindow, GlfwWindowDeleter>;