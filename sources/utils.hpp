#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>

inline std::string loadFile(const std::filesystem::path& filePath)
{
    auto file = std::ifstream { filePath };
    if (!file.is_open()) {
        throw std::runtime_error { "Failed to open file: " + filePath.string() };
    }
    auto sstr = std::stringstream { };
    sstr << file.rdbuf();
    return sstr.str();
}

struct GlfwContext {
    GlfwContext()
    {
        if (!glfwInit()) {
            throw std::runtime_error { "Failed to init GLFW" };
        }
    }

    ~GlfwContext() { glfwTerminate(); }

    GlfwContext(const GlfwContext&) = delete;
    GlfwContext& operator=(const GlfwContext&) = delete;
    GlfwContext(GlfwContext&&) = delete;
    GlfwContext& operator=(GlfwContext&&) = delete;
};

struct GlfwWindowDeleter {
    void operator()(GLFWwindow* window) const
    {
        if (window != nullptr)
            glfwDestroyWindow(window);
    }
};

using GlfwWindowPtr = std::unique_ptr<GLFWwindow, GlfwWindowDeleter>;

template <typename T, typename Deleter>
class GlHandle {
public:
    GlHandle() = default;
    explicit GlHandle(T id)
        : id(id)
    {
    }

    ~GlHandle() { reset(); }

    GlHandle(const GlHandle&) = delete;
    GlHandle& operator=(const GlHandle&) = delete;

    GlHandle(GlHandle&& other) noexcept
        : id(other.release())
    {
    }

    GlHandle& operator=(GlHandle&& other) noexcept
    {
        if (this != &other)
            reset(other.release());
        return *this;
    }

    T get() const { return id; }

    T release()
    {
        T tmp = id;
        id = 0;
        return tmp;
    }

    void reset(T newId = 0)
    {
        if (id)
            Deleter { }(id);
        id = newId;
    }

private:
    T id = 0;
};

struct GlShaderDeleter {
    void operator()(GLuint id) const noexcept { glDeleteShader(id); }
};
using GlShader = GlHandle<GLuint, GlShaderDeleter>;

struct GlProgramDeleter {
    void operator()(GLuint id) const noexcept { glDeleteProgram(id); }
};
using GlProgram = GlHandle<GLuint, GlProgramDeleter>;

struct GlBufferDeleter {
    void operator()(GLuint id) const noexcept { glDeleteBuffers(1, &id); }
};
using GlBuffer = GlHandle<GLuint, GlBufferDeleter>;

struct GlVertexArrayDeleter {
    void operator()(GLuint id) const noexcept { glDeleteVertexArrays(1, &id); }
};
using GlVertexArray = GlHandle<GLuint, GlVertexArrayDeleter>;

struct GlTextureDeleter {
    void operator()(GLuint id) const noexcept { glDeleteTextures(1, &id); }
};
using GlTexture = GlHandle<GLuint, GlTextureDeleter>;

struct GlFramebufferDeleter {
    void operator()(GLuint id) const noexcept { glDeleteFramebuffers(1, &id); }
};
using GlFramebuffer = GlHandle<GLuint, GlFramebufferDeleter>;

struct GlRenderbufferDeleter {
    void operator()(GLuint id) const noexcept { glDeleteRenderbuffers(1, &id); }
};
using GlRenderbuffer = GlHandle<GLuint, GlRenderbufferDeleter>;

struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 tex_coords;
};
