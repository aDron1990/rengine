#pragma once

#include "graphics/Buffer.hpp"
#include "graphics/RenderBackend.hpp"
#include "graphics/Shader.hpp"
#include "graphics/VertexArray.hpp"

#include <entt/entity/fwd.hpp>
#include <entt/entt.hpp>
#include <memory>
#include <sys/stat.h>

class RenderSystem {
public:
    RenderSystem(entt::registry& registry);

    void render(const glm::mat4& proj) noexcept;
    Shader& getShader() noexcept { return m_shader; }

private:
    entt::registry& m_registry;

    std::shared_ptr<RenderBackend> m_backend;

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