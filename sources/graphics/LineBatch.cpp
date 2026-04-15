#include "LineBatch.hpp"
#include "Buffer.hpp"
#include "Input.hpp"
#include "VertexArray.hpp"

void LineBatch::push(const Line& line) noexcept
{
    m_lines.push_back(line);
}

void LineBatch::draw() const noexcept
{
    VertexArray vao;
    vao.bind();
    Buffer<GL_ARRAY_BUFFER, GL_STREAM_DRAW> vertices { m_lines.data(), m_lines.size() * sizeof(Line) };

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float),
        (void*)0);
    glEnableVertexAttribArray(0);

    glDrawArrays(GL_LINES, 0, m_lines.size() * 2);
}