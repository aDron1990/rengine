#include "RenderTexture.hpp"
#include "utils.hpp"

RenderTexture::RenderTexture(const glm::ivec2& size)
    : m_size { size }
{
    resize(size);
}

void RenderTexture::resize(const glm::ivec2& size)
{
    reset();
    create(size);
}

void RenderTexture::reset() noexcept
{
    m_fbo.reset();
    m_depth.reset();
}

void RenderTexture::create(const glm::ivec2& size) noexcept
{
    GLuint texture;
    glGenTextures(1, &texture);
    m_texture.reset(texture);
    bind();
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, size.x, size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    GLuint fbo;
    glGenFramebuffers(1, &fbo);
    m_fbo.reset(fbo);
    bindFBO();
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_texture.get(), 0);

    GLuint depth;
    glGenRenderbuffers(1, &depth);
    m_depth.reset(depth);
    glBindRenderbuffer(GL_RENDERBUFFER, m_depth.get());
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, size.x, size.y);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_depth.get());
    unbindFBO();
}

void RenderTexture::bindFBO() const noexcept
{
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo.get());
}

void RenderTexture::unbindFBO() const noexcept
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}