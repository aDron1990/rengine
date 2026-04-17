#include "OglRenderBackend.hpp"
#include "OglCubemap.hpp"
#include "OglMesh.hpp"
#include "OglPipeline.hpp"
#include "OglShader.hpp"
#include "OglTexture.hpp"
#include "graphics/Buffer.hpp"
#include "graphics/VertexArray.hpp"
#include "graphics/opengl/OglRenderTexture.hpp"

#include <GL/glew.h>
#include <cassert>

OglRenderBackend::OglRenderBackend(glm::ivec2 windowSize)
    : m_windowSize(windowSize)
{
}

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
    m_textures.emplace_back(OglTexture { image });
    return m_textures.size() - 1;
}

PipelineID OglRenderBackend::createPipeline(const PipelineParams& params, const RenderState& state) noexcept
{
    OglShader shader { params.vertexPath, params.fragmentPath };
    OglPipeline pipeline { params, state, std::move(shader) };
    m_pipelines.emplace_back(std::move(pipeline));
    return m_pipelines.size() - 1;
}

CubemapID OglRenderBackend::createCubemap(const std::vector<Image>& faces) noexcept
{
    OglCubemap cubemap;

    cubemap.vao.bind();
    cubemap.vbo = VertexBuffer { cubemapVertices, sizeof(cubemapVertices) };
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float),
        (void*)0);
    glEnableVertexAttribArray(0);

    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
    cubemap.texture.reset(textureID);

    int i = 0;
    for (const auto& face : faces) {
        auto format = face.channels == 3 ? GL_RGB : GL_RGBA;
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, face.width, face.height, 0, format, GL_UNSIGNED_BYTE, face.data.data());
        i++;
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    m_cubemaps.emplace_back(std::move(cubemap));
    return m_cubemaps.size() - 1;
}

RenderTextureID OglRenderBackend::createRenderTexture(uint32_t width, uint32_t height) noexcept
{
    m_renderTextures.emplace_back(OglRenderTexture { { width, height } });
    return m_renderTextures.size() - 1;
}

glm::ivec2 OglRenderBackend::getRenderTextureSize(RenderTextureID texture) noexcept
{
    assert(m_renderTextures.size() > texture);
    return m_renderTextures[texture].getSize();
}

size_t OglRenderBackend::getGuiTexture(RenderTextureID texture) noexcept
{
    assert(m_renderTextures.size() > texture);
    return (size_t)m_renderTextures[texture].getTexture();
}

void OglRenderBackend::bindTexture(TextureID texture, int slot) noexcept
{
    assert(m_textures.size() > texture);
    m_textures[texture].bind(slot);
}

void OglRenderBackend::bindRenderTexture(RenderTextureID texture, int slot) noexcept
{
    assert(m_renderTextures.size() > texture);
    m_renderTextures[texture].bind(slot);
}

void OglRenderBackend::bindFramebuffer(RenderTextureID texture) noexcept
{
    assert(m_renderTextures.size() > texture);
    m_renderTextures[texture].bindFBO();
}

void OglRenderBackend::bindDefaultFramebuffer() noexcept
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, m_windowSize.x, m_windowSize.y);
}

void OglRenderBackend::bindPipeline(PipelineID pipeline) noexcept
{
    assert(m_pipelines.size() > pipeline);
    auto& pipe = m_pipelines[pipeline];
    pipe.shader.use();
    applyState(pipe.state);
}

void OglRenderBackend::resizeDefaultFramebuffer(uint32_t width, uint32_t height) noexcept
{
    m_windowSize = { width, height };
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

void OglRenderBackend::drawCubemap(CubemapID cubemap) noexcept
{
    assert(m_cubemaps.size() > cubemap);
    m_cubemaps[cubemap].vao.bind();
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_cubemaps[cubemap].texture.get());
    glDrawArrays(GL_TRIANGLES, 0, 36);
}

void OglRenderBackend::clearColor() noexcept
{
    glClear(GL_COLOR_BUFFER_BIT);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
}

void OglRenderBackend::clearDepth() noexcept
{
    glClear(GL_DEPTH_BUFFER_BIT);
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