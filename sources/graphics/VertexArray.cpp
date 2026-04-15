#include "VertexArray.hpp"

VertexArray::VertexArray()
{
    GLuint vao;
    glGenVertexArrays(1, &vao);
    m_vao.reset(vao);
}

void VertexArray::bind() const noexcept { glBindVertexArray(m_vao.get()); }

void VertexArray::unbind() const noexcept { glBindVertexArray(0); }
