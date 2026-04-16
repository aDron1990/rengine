#include "OglRenderBackend.hpp"
#include "OglMesh.hpp"
#include "OglPipeline.hpp"
#include "OglShader.hpp"
#include "OglTexture.hpp"
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

TextureID OglRenderBackend::createTexture(const Image& image) noexcept
{
    OglTexture texture;
    GLuint glTexture;
    glGenTextures(1, &glTexture);
    texture.texture.reset(glTexture);

    glBindTexture(GL_TEXTURE_2D, texture.texture.get());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
        GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image.width, image.height, 0, GL_RGBA,
        GL_UNSIGNED_BYTE, image.data.data());
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);

    m_textures.emplace_back(std::move(texture));
    return m_textures.size() - 1;
}

PipelineID OglRenderBackend::createPipeline(const PipelineParams& params, const RenderState& state) noexcept
{
    OglShader shader { params.vertexPath, params.fragmentPath };
    OglPipeline pipeline { params, state, std::move(shader) };
    m_pipelines.emplace_back(std::move(pipeline));
    return m_pipelines.size() - 1;
}

void OglRenderBackend::bindTexture(TextureID texture, int slot) noexcept
{
    assert(m_textures.size() > texture);
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D, m_textures[texture].texture.get());
}

void OglRenderBackend::bindPipeline(PipelineID pipeline) noexcept
{
    assert(m_pipelines.size() > pipeline);
    auto& pipe = m_pipelines[pipeline];
    pipe.shader.use();
    applyState(pipe.state);
}

void OglRenderBackend::drawMesh(MeshID mesh) noexcept
{
    assert(m_meshes.size() > mesh);
    m_meshes[mesh].vao.bind();
    glDrawElements(GL_TRIANGLES, m_meshes[mesh].indicesCount, GL_UNSIGNED_INT, nullptr);
}

void OglRenderBackend::drawLines(const std::vector<Line>& lines) noexcept
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

void OglRenderBackend::setValue(PipelineID pipeline, const std::string& name, Value value) noexcept
{
    auto& shader = m_pipelines[pipeline].shader;
    std::visit([&](auto&& v) {
        using T = std::decay_t<decltype(v)>;
        shader.setUniform<T>(v, name);
    },
        value);
}

void OglRenderBackend::applyState(const RenderState& state) noexcept
{
    if (state.cull.mode == Cull::Mode::None)
        glDisable(GL_CULL_FACE);
    else {
        glEnable(GL_CULL_FACE);
        glCullFace(state.cull.mode == Cull::Mode::Front ? GL_FRONT : GL_BACK);
        glFrontFace(state.cull.front == Cull::Front::CW ? GL_CW : GL_CCW);
    }

    if (state.depth) {
        glEnable(GL_DEPTH_TEST);
        glDepthMask(GL_TRUE);
        glDepthFunc(GL_LESS);
    } else {
        glDisable(GL_DEPTH_TEST);
    }

    if (state.blend) {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    } else {
        glDisable(GL_BLEND);
    }
}