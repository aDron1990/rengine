#include "OglTexture.hpp"
#include "graphics/GlHandle.hpp"

OglTexture::OglTexture(const Image& image)
{
    GLuint glTexture;
    glGenTextures(1, &glTexture);
    m_texture.reset(glTexture);

    bind();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
        GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image.width, image.height, 0, GL_RGBA,
        GL_UNSIGNED_BYTE, image.data.data());
    glGenerateMipmap(GL_TEXTURE_2D);
    unbind();
}

void OglTexture::bind(int slot) noexcept
{
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D, m_texture.get());
}

void OglTexture::unbind() noexcept
{
    glBindTexture(GL_TEXTURE_2D, 0);
}
