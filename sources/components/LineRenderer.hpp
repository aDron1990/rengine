#pragma once

#include "graphics/RenderLayer.hpp"
#include "graphics/RenderPass.hpp"
#include "graphics/types.hpp"
#include "utils/types.hpp"

#include <unordered_map>
#include <vector>

struct LineRenderer {
    std::vector<Line> lines;
    glm::vec4 color = { 0.0f, 1.0f, 0.0f, 1.0f };
    int layer = DEFAULT_RENDER_LAYER;
    bool draw = true;
};

class LinePass : public RenderPass {
public:
    LinePass(PipelineID pipeline);
    void collect(const RenderContext& ctx) noexcept override;
    void render(RenderBackend& backend, const RenderContext& ctx) noexcept override;

private:
    void renderLayer(int layer, const std::vector<entt::entity> entities, RenderBackend& backend, RenderContext ctx) noexcept;
    void renderLines(const std::vector<entt::entity> entities, entt::entity cameraEntity, RenderBackend& backend, RenderContext ctx) noexcept;

private:
    PipelineID m_pipeline;
    std::unordered_map<int, std::vector<entt::entity>> m_layers;
};
