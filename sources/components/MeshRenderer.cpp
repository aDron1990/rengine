#include "MeshRenderer.hpp"
#include "Camera.hpp"
#include "Frustum.hpp"
#include "graphics/RenderLayer.hpp"

#include <entt/entt.hpp>

MeshPass::MeshPass(PipelineID pipeline)
    : m_pipeline { pipeline }
{
}

void MeshPass::collect(const RenderContext& ctx) noexcept
{
    auto view = ctx.registry.view<MeshRenderer>();
    for (auto [entity, renderer] : view->each()) {
        m_layers[renderer.layer].emplace_back(entity);
    }
}

void MeshPass::render(RenderBackend& backend, const RenderContext& ctx) noexcept
{
    for (auto layer : m_layers) {
        renderLayer(layer.first, layer.second, backend, ctx);
    }
    m_layers.clear();
}

void MeshPass::renderLayer(int nlayer, const std::vector<entt::entity> entities, RenderBackend& backend, RenderContext ctx) noexcept
{
    entt::entity cameraEntity;
    auto size = ctx.defaultLayerSize;

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

    backend.clearDepth();

    renderMeshes(entities, cameraEntity, backend, ctx);
}

void MeshPass::renderMeshes(const std::vector<entt::entity> entities, entt::entity cameraEntity, RenderBackend& backend, RenderContext ctx) noexcept
{
    auto [camera, cameraTransform] = ctx.registry.get<Camera, Transform>(cameraEntity);

    backend.bindPipeline(m_pipeline);
    backend.setValue(m_pipeline, "view", ctx.view);
    backend.setValue(m_pipeline, "proj", ctx.proj);

    Frustum frustum { ctx.proj * ctx.view };

    glm::vec3 lightPos { -15.0f, 15.0f, 15.0f };
    backend.setValue(m_pipeline, "light.position", lightPos);
    backend.setValue(m_pipeline, "light.ambient", glm::vec3 { 0.2f, 0.2f, 0.2f });
    backend.setValue(m_pipeline, "light.diffuse", glm::vec3 { 0.5f, 0.5f, 0.5f });
    backend.setValue(m_pipeline, "light.specular", glm::vec3 { 1.0f, 1.0f, 1.0f });
    backend.setValue(m_pipeline, "viewPos", cameraTransform.position);

    int drawed = 0;
    for (auto entity : entities) {
        auto [transform, renderer, bb] = ctx.registry.get<Transform, MeshRenderer, BoundingBox>(entity);

        auto aabb = toGlobalAABB(bb, transform);
        if (!frustum.isVisible(aabb))
            continue;
        drawed++;

        auto model = transform.getMatrix();
        backend.setValue(m_pipeline, "model", model);
        backend.setValue(m_pipeline, "material.ambient", glm::vec3 { 0.8f, 0.8f, 0.8f });
        backend.setValue(m_pipeline, "material.diffuse", 0);
        backend.setValue(m_pipeline, "material.specular", 1);
        backend.setValue(m_pipeline, "material.shininess", 32.0f);

        backend.bindTexture(renderer.texture, 0);
        backend.bindTexture(renderer.specular, 1);

        auto& meshes = renderer.model->getMeshes();
        for (auto& mesh : meshes) {
            backend.drawMesh(mesh.meshID);
        }
    }
}