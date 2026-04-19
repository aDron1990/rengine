#pragma once

#include "Model.hpp"
#include "graphics/RenderContext.hpp"
#include "graphics/RenderLayer.hpp"
#include "graphics/RenderPass.hpp"
#include "graphics/types.hpp"

#include <entt/entity/fwd.hpp>
#include <memory>
#include <unordered_map>
#include <vector>

struct MeshRenderer {
    std::shared_ptr<Model> model;
    TextureID texture;
    TextureID specular;
    glm::vec3 ambient = { 0.8f, 0.8f, 0.8f };
    float shininess = 32.0f;
    bool shaded = true;
    int layer = DEFAULT_RENDER_LAYER;
};

class MeshPass : public RenderPass {
public:
    MeshPass(PipelineID shadedMeshPipeline, PipelineID meshPipeline);
    void collect(const RenderContext& ctx) noexcept override;
    void render(RenderBackend& backend, const RenderContext& ctx) noexcept override;

private:
    void renderLayer(int layer, const std::vector<entt::entity> entities, PipelineID pipeline, RenderBackend& backend, RenderContext ctx) noexcept;
    void renderMeshes(const std::vector<entt::entity> entities, entt::entity cameraEntity, PipelineID pipeline, RenderBackend& backend, RenderContext ctx) noexcept;

private:
    PipelineID m_shadedMeshPipeline;
    PipelineID m_meshPipeline;
    std::unordered_map<int, std::unordered_map<bool, std::vector<entt::entity>>> m_layers;
};
