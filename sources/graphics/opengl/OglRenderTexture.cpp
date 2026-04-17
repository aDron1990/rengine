#include "OglRenderTexture.hpp"

OglRenderTexture::OglRenderTexture(const glm::ivec2& size)
{
    resize(size);
}

void OglRenderTexture::resize(const glm::ivec2& size)
{
    reset();
    create(size);
}

void OglRenderTexture::reset() noexcept
{
    m_texture.reset();
    m_fbo.reset();
    m_depth.reset();
}

void OglRenderTexture::create(const glm::ivec2& size) noexcept
{
    m_width = size.x;
    m_height = size.y;

    GLuint tex;
    glGenTextures(1, &tex);
    m_texture.reset(tex);
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
}

void OglRenderTexture::bindFBO() const noexcept
{
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo.get());
    glViewport(0, 0, getSize().x, getSize().y);
}

void OglRenderTexture::unbindFBO() const noexcept
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}