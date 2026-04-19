#pragma once

#include "graphics/RenderLayer.hpp"
#include "graphics/RenderPass.hpp"
#include "graphics/types.hpp"
#include <entt/entity/fwd.hpp>
#include <unordered_map>

struct SkyboxRenderer {
    CubemapID cubemap;
    int layer = DEFAULT_RENDER_LAYER;
};

class SkyboxPass : public RenderPass {
public:
    SkyboxPass(PipelineID pipeline);
    void collect(const RenderContext& ctx) noexcept override;
    void render(RenderBackend& backend, const RenderContext& ctx) noexcept override;

private:
    void renderLayer(int nlayer, entt::entity entity, RenderBackend& backend, const RenderContext& ctx) noexcept;

private:
    PipelineID m_pipeline;
    std::unordered_map<int, entt::entity> m_layers;
};