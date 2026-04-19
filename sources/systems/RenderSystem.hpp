#pragma once

#include "graphics/RenderBackend.hpp"
#include "graphics/RenderLayer.hpp"
#include "graphics/RenderPass.hpp"
#include "graphics/types.hpp"

#include <cstdint>
#include <entt/entity/fwd.hpp>
#include <entt/entt.hpp>
#include <imgui.h>
#include <memory>
#include <sys/stat.h>
#include <vector>

class RenderSystem {
public:
    RenderSystem(entt::registry& registry, uint32_t width, uint32_t height);
    void resize(uint32_t width, uint32_t height) noexcept;
    void render() noexcept;

    int addRenderLayer(uint32_t width, uint32_t height, entt::entity camera) noexcept;
    void setRenderLayerCamera(int nlayer, entt::entity camera) noexcept;
    ImTextureID getGuiTextureFromLayer(int nlayer) noexcept;

    void addPass(std::unique_ptr<RenderPass> pass) noexcept;

private:
    void renderLayerLines(int nlayer, const std::vector<entt::entity>& entities) noexcept;
    void renderLines(const std::vector<entt::entity>& entities, entt::entity cameraEntity, const glm::mat4& view, const glm::mat4& proj) noexcept;
    void renderCubemap(entt::entity cameraEntity, const glm::mat4& view, const glm::mat4& proj) noexcept;

private:
    entt::registry& m_registry;

    std::shared_ptr<RenderBackend> m_backend;
    std::vector<std::unique_ptr<RenderPass>> m_passes;

    PipelineID m_mainPipe;
    PipelineID m_skyboxPipe;
    PipelineID m_transparentPipe;
    PipelineID m_linesPipe;
    CubemapID m_cubemap;

    entt::entity m_defaultLayerCamera;
    std::vector<RenderLayer> m_layers;
    glm::ivec2 m_size;
};