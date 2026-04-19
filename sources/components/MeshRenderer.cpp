#include "MeshRenderer.hpp"
#include "Camera.hpp"
#include "Frustum.hpp"
#include "graphics/RenderLayer.hpp"
#include "graphics/types.hpp"

#include <entt/entt.hpp>

MeshPass::MeshPass(PipelineID shadedMeshPipeline, PipelineID meshPipeline)
    : m_shadedMeshPipeline { shadedMeshPipeline }
    , m_meshPipeline { meshPipeline }
{
}

void MeshPass::collect(const RenderContext& ctx) noexcept
{
    auto view = ctx.registry.view<MeshRenderer>();
    for (auto [entity, renderer] : view->each()) {
        m_layers[renderer.layer][renderer.shaded].emplace_back(entity);
    }
}

void MeshPass::render(RenderBackend& backend, const RenderContext& ctx) noexcept
{
    for (auto [layer, second] : m_layers) {
        for (auto [shaded, entities] : second) {
            renderLayer(layer, entities, shaded ? m_shadedMeshPipeline : m_meshPipeline, backend, ctx);
        }
    }
    m_layers.clear();
}

void MeshPass::renderLayer(int nlayer, const std::vector<entt::entity> entities, PipelineID pipeline, RenderBackend& backend, RenderContext ctx) noexcept
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

    renderMeshes(entities, cameraEntity, pipeline, backend, ctx);
}

void MeshPass::renderMeshes(const std::vector<entt::entity> entities, entt::entity cameraEntity, PipelineID pipeline, RenderBackend& backend, RenderContext ctx) noexcept
{
    auto [camera, cameraTransform] = ctx.registry.get<Camera, Transform>(cameraEntity);

    backend.bindPipeline(pipeline);
    backend.setValue(pipeline, "view", ctx.view);
    backend.setValue(pipeline, "proj", ctx.proj);

    Frustum frustum { ctx.proj * ctx.view };

    glm::vec3 lightPos { -15.0f, 15.0f, 15.0f };
    backend.setValue(pipeline, "light.position", lightPos);
    backend.setValue(pipeline, "light.ambient", glm::vec3 { 0.2f, 0.2f, 0.2f });
    backend.setValue(pipeline, "light.diffuse", glm::vec3 { 0.5f, 0.5f, 0.5f });
    backend.setValue(pipeline, "light.specular", glm::vec3 { 1.0f, 1.0f, 1.0f });
    backend.setValue(pipeline, "viewPos", cameraTransform.position);

    int drawed = 0;
    for (auto entity : entities) {
        auto [transform, renderer, bb] = ctx.registry.get<Transform, MeshRenderer, BoundingBox>(entity);

        auto aabb = toGlobalAABB(bb, transform);
        if (!frustum.isVisible(aabb))
            continue;
        drawed++;

        auto model = transform.getMatrix();
        backend.setValue(pipeline, "model", model);
        backend.setValue(pipeline, "material.ambient", renderer.ambient);
        backend.setValue(pipeline, "material.shininess", renderer.shininess);
        backend.setValue(pipeline, "material.diffuse", 0);
        backend.setValue(pipeline, "material.specular", 1);

        backend.bindTexture(renderer.texture, 0);
        backend.bindTexture(renderer.specular, 1);

        auto& meshes = renderer.model->getMeshes();
        for (auto& mesh : meshes) {
            backend.drawMesh(mesh.meshID);
        }
    }
}