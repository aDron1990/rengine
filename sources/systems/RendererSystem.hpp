#pragma once

#include "Buffer.hpp"
#include "Shader.hpp"
#include "VertexArray.hpp"

#include <entt/entity/fwd.hpp>
#include <entt/entt.hpp>
#include <sys/stat.h>

class RendererSystem {
public:
    RendererSystem(entt::registry& registry);

    void render(const glm::mat4& proj) noexcept;
    Shader& getShader() noexcept { return m_shader; }

private:
    entt::registry& m_registry;
    Shader m_shader;
    Shader m_skyboxShader;
    Shader m_transparentShader;
    Shader m_linesShader;
    GlTexture m_skyboxTexture { };
    VertexArray m_skyboxVAO { };
    VertexBuffer m_skyboxVBO { };

    const static int FRAMES = 20;
    int m_currentFrame = 0;
    std::array<float, FRAMES> m_lastFrametimes { };
    float FPS = 0.f;
};