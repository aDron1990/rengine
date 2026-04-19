#include "SkyboxRenderer.hpp"
#include "Camera.hpp"
#include "components/Transform.hpp"
#include "graphics/RenderContext.hpp"

#include <entt/entt.hpp>

SkyboxPass::SkyboxPass(PipelineID pipeline)
    : m_pipeline { pipeline }
{
}

void SkyboxPass::collect(const RenderContext& ctx) noexcept
{
    for (auto [entity, renderer] : ctx.registry.view<SkyboxRenderer>()->each()) {
        m_layers[renderer.layer] = entity;
    }
}

void SkyboxPass::render(RenderBackend& backend, const RenderContext& ctx) noexcept
{
    for (auto [layer, entity] : m_layers) {
        renderLayer(layer, entity, backend, ctx);
    }
    m_layers.clear();
}

void SkyboxPass::renderLayer(int nlayer, entt::entity entity, RenderBackend& backend, const RenderContext& ctx) noexcept
{
    bindLayer(nlayer, backend, ctx);

    backend.clearDepth();
    backend.clearColor();

    auto cameraEntity = getLayerCamera(nlayer, backend, ctx);
    auto size = getLayerSize(nlayer, backend, ctx);

    auto [camera, cameraTrans] = ctx.registry.get<Camera, Transform>(cameraEntity);
    auto view = camera.getView(cameraTrans.position);
    auto proj = camera.getProj((float)size.x / size.y);
    auto view_ = glm::mat4(glm::mat3(view));

    backend.bindPipeline(m_pipeline);
    backend.setValue(m_pipeline, "view", view_);
    backend.setValue(m_pipeline, "proj", proj);
    backend.drawCubemap(ctx.registry.get<SkyboxRenderer>(entity).cubemap);
}