#pragma once

#include "Input.hpp"
#include "utils.hpp"

template <GLenum type, GLenum usage = GL_STATIC_DRAW>
class Buffer {
public:
    Buffer() = default;

    Buffer(const void* data, size_t size)
    {
        GLuint buffer;
        glGenBuffers(1, &buffer);
        m_buffer.reset(buffer);
        bind();
        glBufferData(type, size, data, usage);
    }

    void bind() const noexcept { glBindBuffer(type, m_buffer.get()); }

    void unbind() const noexcept { glBindBuffer(type, 0); }

private:
    GlBuffer m_buffer;
};

using VertexBuffer = Buffer<GL_ARRAY_BUFFER>;
using IndexBuffer = Buffer<GL_ELEMENT_ARRAY_BUFFER>;
