#pragma once

#include "graphics/RenderBackend.hpp"
#include "graphics/RenderLayer.hpp"
#include "graphics/RenderPass.hpp"
#include "graphics/types.hpp"

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <cstdint>
#include <entt/entity/fwd.hpp>
#include <entt/entt.hpp>
#include <imgui.h>
#include <memory>
#include <sys/stat.h>
#include <vector>

class RenderEngine {
public:
    RenderEngine(entt::registry& registry, uint32_t width, uint32_t height, GLFWwindow* window);
    void resize(uint32_t width, uint32_t height) noexcept;
    void render() noexcept;

    int addRenderLayer(uint32_t width, uint32_t height, entt::entity camera) noexcept;
    void setRenderLayerCamera(int nlayer, entt::entity camera) noexcept;
    ImTextureID getGuiTextureFromLayer(int nlayer) noexcept;

    void addPass(std::unique_ptr<RenderPass> pass) noexcept;

private:
    entt::registry& m_registry;

    std::shared_ptr<RenderBackend> m_backend;
    std::vector<std::unique_ptr<RenderPass>> m_passes;

    PipelineID m_shadedMeshPipe;
    PipelineID m_meshPipe;
    PipelineID m_skyboxPipe;
    PipelineID m_transparentPipe;
    PipelineID m_linesPipe;
    CubemapID m_cubemap;

    entt::entity m_defaultLayerCamera;
    std::vector<RenderLayer> m_layers;
    glm::ivec2 m_size;
};