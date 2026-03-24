#include "RendererSystem.hpp"

#include "Cubemap.hpp"

RendererSystem::RendererSystem(entt::registry& registry, Camera& camera)
    : m_registry { registry }
    , m_shader { "resources/shaders/main_v.glsl", "resources/shaders/main_f.glsl" }
    , m_skyboxShader { "resources/shaders/cubemap_v.glsl", "resources/shaders/cubemap_f.glsl" }
    , m_camera { camera }
{
    // Skybox
    m_skyboxTexture.reset(
        loadCubemap({ "resources/images/skybox/right.jpg",
            "resources/images/skybox/left.jpg",
            "resources/images/skybox/top.jpg",
            "resources/images/skybox/bottom.jpg",
            "resources/images/skybox/front.jpg",
            "resources/images/skybox/back.jpg" }));

    m_skyboxVAO.bind();
    m_skyboxVBO = VertexBuffer { skyboxVertices, sizeof(skyboxVertices) };
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float),
        (void*)0);
    glEnableVertexAttribArray(0);
}

void RendererSystem::render(const glm::mat4& view, const glm::mat4& proj) noexcept
{
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    auto view_ = glm::mat4(glm::mat3(view));
    glDepthMask(GL_FALSE);
    m_skyboxShader.use();
    m_skyboxShader.setUniform(view_, "view");
    m_skyboxShader.setUniform(proj, "proj");
    m_skyboxVAO.bind();
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_skyboxTexture.get());
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glDepthMask(GL_TRUE);
}
