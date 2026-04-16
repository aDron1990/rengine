#include "OglRenderBackend.hpp"
#include "OglMesh.hpp"
#include "graphics/Buffer.hpp"
#include "graphics/VertexArray.hpp"

#include <GL/glew.h>
#include <cassert>

MeshID OglRenderBackend::createMesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices) noexcept
{
    OglMesh mesh;
    mesh.verticesCount = vertices.size();
    mesh.indicesCount = indices.size();

    mesh.vao.bind();
    mesh.vbo = VertexBuffer {
        vertices.data(),
        vertices.size() * sizeof(Vertex)
    };
    mesh.ibo = IndexBuffer {
        indices.data(),
        indices.size() * sizeof(uint32_t)
    };
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
        sizeof(Vertex), (void*)offsetof(Vertex, position));
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,
        sizeof(Vertex), (void*)offsetof(Vertex, normal));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE,
        sizeof(Vertex), (void*)offsetof(Vertex, tex_coords));
    glEnableVertexAttribArray(2);
    mesh.vao.unbind();

    m_meshes.emplace_back(std::move(mesh));
    return m_meshes.size() - 1;
}

void OglRenderBackend::draw(MeshID mesh) noexcept
{
    assert(m_meshes.size() > mesh);
    m_meshes[mesh].vao.bind();
    glDrawElements(GL_TRIANGLES, m_meshes[mesh].indicesCount, GL_UNSIGNED_INT, nullptr);
}

void OglRenderBackend::draw(const std::vector<Line>& lines) noexcept
{
    VertexArray vao;
    vao.bind();
    Buffer<GL_ARRAY_BUFFER, GL_STREAM_DRAW> vertices { lines.data(), lines.size() * sizeof(Line) };

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float),
        (void*)0);
    glEnableVertexAttribArray(0);

    glDrawArrays(GL_LINES, 0, lines.size() * 2);
}

void OglRenderBackend::clear() noexcept
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
}