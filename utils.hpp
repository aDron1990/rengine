#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <stdexcept>
#include <memory>
#include <string>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <iostream>

inline std::string loadFile(const std::filesystem::path& filePath) {
    auto file = std::ifstream{ filePath };
    if (!file.is_open()) {
        throw std::runtime_error{ "Failed to open file: " + filePath.string() };
    }
    auto sstr = std::stringstream{};
    sstr << file.rdbuf();
    return sstr.str();
}

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

template<typename T, typename Deleter>
class GLHandle {
public:
    GLHandle() = default;
    explicit GLHandle(T id) : id(id) {}

    ~GLHandle() { reset(); }

    GLHandle(const GLHandle&) = delete;
    GLHandle& operator=(const GLHandle&) = delete;

    GLHandle(GLHandle&& other) noexcept : id(other.release()) {}

    GLHandle& operator=(GLHandle&& other) noexcept {
        if (this != &other)
            reset(other.release());
        return *this;
    }

    T get() const { return id; }

    T release() {
        T tmp = id;
        id = 0;
        return tmp;
    }

    void reset(T newId = 0) {
        if (id)
            Deleter{}(id);
        id = newId;
    }

private:
    T id = 0;
};

struct GlShaderDeleter {
    void operator()(GLuint id) const noexcept
    {
        glDeleteShader(id);
    }
};
using GlShader = GLHandle<GLuint, GlShaderDeleter>;

struct GlProgramDeleter {
	void operator()(GLuint id) const noexcept
	{
		glDeleteProgram(id);
	}
};
using GlProgram = GLHandle<GLuint, GlProgramDeleter>;
