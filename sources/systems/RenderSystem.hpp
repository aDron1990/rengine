#pragma once

#include "graphics/Buffer.hpp"
#include "graphics/RenderBackend.hpp"
#include "graphics/VertexArray.hpp"
#include "graphics/types.hpp"

#include <entt/entity/fwd.hpp>
#include <entt/entt.hpp>
#include <memory>
#include <sys/stat.h>

class RenderSystem {
public:
    RenderSystem(entt::registry& registry);

    void render(const glm::mat4& proj) noexcept;

private:
    entt::registry& m_registry;

    std::shared_ptr<RenderBackend> m_backend;

    PipelineID m_mainPipe;
    PipelineID m_skyboxPipe;
    PipelineID m_transparentPipe;
    PipelineID m_linesPipe;
    GlTexture m_skyboxTexture { };
    VertexArray m_skyboxVAO { };
    VertexBuffer m_skyboxVBO { };

    const static int FRAMES = 20;
    int m_currentFrame = 0;
    std::array<float, FRAMES> m_lastFrametimes { };
    float FPS = 0.f;
};