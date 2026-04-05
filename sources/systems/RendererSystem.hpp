#pragma once

#include "Buffer.hpp"
#include "Camera.hpp"
#include "Shader.hpp"
#include "VertexArray.hpp"

#include <entt/entt.hpp>

class RendererSystem {
public:
    RendererSystem(entt::registry& registry, Camera& camera);

    void render(const glm::mat4& view, const glm::mat4& proj) noexcept;
    Shader& getShader() noexcept { return m_shader; }

private:
    entt::registry& m_registry;
    Shader m_shader;
    Shader m_skyboxShader;
    Shader m_transparentShader;
    Shader m_linesShader;
    Camera& m_camera;
    GlTexture m_skyboxTexture { };
    VertexArray m_skyboxVAO { };
    VertexBuffer m_skyboxVBO { };
};