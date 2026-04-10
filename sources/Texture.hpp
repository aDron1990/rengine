#pragma once

#include "utils.hpp"

class Texture {
public:
    Texture() = default;
    Texture(const std::filesystem::path& filePath);

    void bind(int slot = 0) const noexcept;
    void unbind() const noexcept;
    GLuint getId() const noexcept { return m_texture.get(); }

protected:
    GlTexture m_texture;
};
