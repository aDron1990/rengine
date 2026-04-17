#pragma once

#include "../GlHandle.hpp"
#include "graphics/Image.hpp"

#include <cstdint>

class OglTexture {
public:
    OglTexture() = default;
    OglTexture(const Image& image);
    void bind(int slot = 0) noexcept;
    void unbind() noexcept;
    GLuint getTexture() const noexcept { return m_texture.get(); };

protected:
    GlTexture m_texture;
    uint32_t m_width;
    uint32_t m_height;
};
