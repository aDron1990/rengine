#pragma once

#include "../GlHandle.hpp"
#include "OglRenderTarget.hpp"
#include "graphics/opengl/OglTexture.hpp"
#include <glm/ext/vector_int2.hpp>

class OglRenderTexture : public OglTexture, public OglRenderTarget {
public:
    OglRenderTexture(const glm::ivec2& size);
    void resize(const glm::ivec2& size);
    void bindFBO() const noexcept;
    void unbindFBO() const noexcept;
    Type getType() const noexcept override { return OglRenderTarget::Type::Texture; }
    glm::ivec2 getSize() const noexcept override { return { m_width, m_height }; }

private:
    void reset() noexcept;
    void create(const glm::ivec2& size) noexcept;

private:
    GlFramebuffer m_fbo;
    GlRenderbuffer m_depth;
};
