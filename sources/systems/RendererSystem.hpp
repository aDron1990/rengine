#pragma once

#include "Buffer.hpp"
#include "Camera.hpp"
#include "Shader.hpp"
#include "VertexArray.hpp"

#include <chrono>
#include <entt/entt.hpp>
#include <sys/stat.h>

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

    const static int FRAMES = 20;
    int m_currentFrame = 0;
    std::array<float, FRAMES> m_lastFrametimes { };

    std::chrono::steady_clock::time_point m_lastFrame = std::chrono::steady_clock::now();
    float FPS = 0.f;
};