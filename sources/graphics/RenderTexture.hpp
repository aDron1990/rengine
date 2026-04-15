#pragma once

#include "RenderTarget.hpp"
#include "Texture.hpp"
#include "utils.hpp"
#include <glm/ext/vector_int2.hpp>

class RenderTexture : public Texture, public RenderTarget {
public:
    RenderTexture(const glm::ivec2& size);
    void resize(const glm::ivec2& size);
    void bindFBO() const noexcept;
    void unbindFBO() const noexcept;
    Type getType() const noexcept override { return RenderTarget::Type::Texture; }
    glm::ivec2 getSize() const noexcept override { return m_size; }

private:
    void reset() noexcept;
    void create(const glm::ivec2& size) noexcept;

private:
    GlFramebuffer m_fbo;
    GlRenderbuffer m_depth;
    glm::ivec2 m_size;
};
