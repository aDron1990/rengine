#include "LineRenderer.hpp"
#include "Camera.hpp"
#include "components/Transform.hpp"
#include "graphics/RenderContext.hpp"

#include <entt/entt.hpp>

LinePass::LinePass(PipelineID pipeline)
    : m_pipeline { pipeline }
{
}

void LinePass::collect(const RenderContext& ctx) noexcept
{
    auto view = ctx.registry.view<LineRenderer>();
    for (auto [entity, renderer] : view->each()) {
        if (renderer.draw)
            m_layers[renderer.layer].emplace_back(entity);
    }
}

void LinePass::render(RenderBackend& backend, const RenderContext& ctx) noexcept
{
    for (auto [layer, entities] : m_layers) {
        renderLayer(layer, entities, backend, ctx);
    }
    m_layers.clear();
}

void LinePass::renderLayer(int nlayer, const std::vector<entt::entity> entities, RenderBackend& backend, RenderContext ctx) noexcept
{
    bindLayer(nlayer, backend, ctx);
    auto cameraEntity = getLayerCamera(nlayer, backend, ctx);
    auto size = getLayerSize(nlayer, backend, ctx);

    auto [camera, transform] = ctx.registry.get<Camera, Transform>(cameraEntity);
    ctx.view = camera.getView(transform.position);
    ctx.proj = camera.getProj((float)size.x / size.y);

    renderLines(entities, cameraEntity, backend, ctx);
}

void LinePass::renderLines(const std::vector<entt::entity> entities, entt::entity cameraEntity, RenderBackend& backend, RenderContext ctx) noexcept
{
    auto& commandBuffer = backend.getCommandBuffer();
    auto [camera, cameraTransform] = ctx.registry.get<Camera, Transform>(cameraEntity);

    commandBuffer.bindPipeline(m_pipeline);
    commandBuffer.setValue(m_pipeline, "view", ctx.view);
    commandBuffer.setValue(m_pipeline, "proj", ctx.proj);

    for (auto entity : entities) {
        auto& renderer = ctx.registry.get<LineRenderer>(entity);
        commandBuffer.setValue(m_pipeline, "color", renderer.color);
        commandBuffer.drawLines(renderer.lines);
    }
}
