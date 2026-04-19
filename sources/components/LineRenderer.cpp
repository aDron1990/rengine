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

    if (nlayer == DEFAULT_RENDER_LAYER) {
        backend.bindDefaultFramebuffer();
        cameraEntity = ctx.defaultLayerCamera;

    } else {
        auto& layer = ctx.layers[nlayer];
        backend.bindFramebuffer(layer.texture);
        cameraEntity = layer.camera;
        size = backend.getRenderTextureSize(layer.texture);
    }

    auto [camera, transform] = ctx.registry.get<Camera, Transform>(cameraEntity);
    ctx.view = camera.getView(transform.position);
    ctx.proj = camera.getProj((float)size.x / size.y);

    renderLines(entities, cameraEntity, backend, ctx);
}

void LinePass::renderLines(const std::vector<entt::entity> entities, entt::entity cameraEntity, RenderBackend& backend, RenderContext ctx) noexcept
{
    auto [camera, cameraTransform] = ctx.registry.get<Camera, Transform>(cameraEntity);

    backend.bindPipeline(m_pipeline);
    backend.setValue(m_pipeline, "view", ctx.view);
    backend.setValue(m_pipeline, "proj", ctx.proj);

    for (auto entity : entities) {
        auto& renderer = ctx.registry.get<LineRenderer>(entity);
        backend.setValue(m_pipeline, "color", renderer.color);
        backend.drawLines(renderer.lines);
    }
}