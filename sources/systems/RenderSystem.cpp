#include "RenderSystem.hpp"
#include "BoundingBox.hpp"
#include "Frustum.hpp"
#include "components/Camera.hpp"
#include "components/MeshRenderer.hpp"
#include "components/Transform.hpp"
#include "graphics/RenderBackend.hpp"
#include "graphics/opengl/OglRenderBackend.hpp"
#include "utils/utils.hpp"

#include <cassert>
#include <entt/entity/fwd.hpp>
#include <imgui.h>
#include <unordered_map>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/ext/matrix_transform.hpp>
#include <glm/geometric.hpp>
#include <glm/gtx/norm.hpp>

#include <array>
#include <memory>
#include <vector>

RenderSystem::RenderSystem(entt::registry& registry, uint32_t width, uint32_t height)
    : m_registry { registry }
    , m_size { width, height }
{
    m_backend.reset(new OglRenderBackend { { width, height } });
    m_registry.ctx().emplace<std::shared_ptr<RenderBackend>>(m_backend);

    m_mainPipe = m_backend->createPipeline({ "resources/shaders/main_v.glsl", "resources/shaders/main_f.glsl" }, RenderState { });
    m_transparentPipe = m_backend->createPipeline({ "resources/shaders/transparent_v.glsl", "resources/shaders/transparent_f.glsl" }, RenderState { });
    m_skyboxPipe = m_backend->createPipeline({ "resources/shaders/cubemap_v.glsl", "resources/shaders/cubemap_f.glsl" }, RenderState { .depth = false });
    m_linesPipe = m_backend->createPipeline({ "resources/shaders/line_v.glsl", "resources/shaders/line_f.glsl" }, RenderState { .depth = false });

    auto cubeImages = loadCubeImages("resources/images/space_skybox");
    m_cubemap = m_backend->createCubemap(cubeImages);
}

void RenderSystem::resize(uint32_t width, uint32_t height) noexcept
{
    m_backend->resizeDefaultFramebuffer(width, height);
    m_size = { width, height };
}

std::array<Line, 12> toLines(const BoundingBox& aabb, const Transform& transform) noexcept;
std::array<Line, 12> toLinesAligned(const BoundingBox& aabb, const Transform& transform) noexcept;

int RenderSystem::addRenderLayer(uint32_t width, uint32_t height, entt::entity camera) noexcept
{
    auto texture = m_backend->createRenderTexture(width, height);
    m_layers.emplace_back(RenderLayer { texture, camera });
    return m_layers.size() - 1;
}

void RenderSystem::setRenderLayerCamera(int nlayer, entt::entity camera) noexcept
{
    if (nlayer == DEFAULT_RENDER_LAYER) {
        m_defaultLayerCamera = camera;
        return;
    }
    assert(m_layers.size() > nlayer);
    m_layers[nlayer].camera = camera;
}

ImTextureID RenderSystem::getGuiTextureFromLayer(int nlayer) noexcept
{
    assert(m_layers.size() > nlayer && nlayer != DEFAULT_RENDER_LAYER);
    return (ImTextureID)m_backend->getGuiTexture(m_layers[nlayer].texture);
}

void RenderSystem::render() noexcept
{
    std::unordered_map<int, std::vector<entt::entity>> entityLayers;
    for (auto [entity, renderer] : m_registry.view<MeshRenderer>()->each()) {
        entityLayers[renderer.layer].emplace_back(entity);
    }

    for (auto [layer, entities] : entityLayers) {
        renderLayer(layer, entities);
    }

    m_backend->bindDefaultFramebuffer();
}

void RenderSystem::renderLayer(int nlayer, const std::vector<entt::entity>& entities) noexcept
{
    entt::entity cameraEntity;
    auto size = m_size;

    if (nlayer == DEFAULT_RENDER_LAYER) {
        m_backend->bindDefaultFramebuffer();
        cameraEntity = m_defaultLayerCamera;

    } else {
        auto& layer = m_layers[nlayer];
        m_backend->bindFramebuffer(layer.texture);
        cameraEntity = layer.camera;
        size = m_backend->getRenderTextureSize(layer.texture);
    }

    auto [camera, transform] = m_registry.get<Camera, Transform>(cameraEntity);
    auto view = camera.getView(transform.position);
    auto proj = camera.getProj((float)size.x / size.y);

    m_backend->clearColor();
    m_backend->clearDepth();

    if (nlayer == DEFAULT_RENDER_LAYER)
        renderCubemap(cameraEntity, view, proj);

    renderMeshes(entities, cameraEntity, view, proj);
}

void RenderSystem::renderCubemap(entt::entity cameraEntity, const glm::mat4& view, const glm::mat4& proj) noexcept
{
    auto [camera, cameraTransform] = m_registry.get<Camera, Transform>(cameraEntity);

    auto view_ = glm::mat4(glm::mat3(view));
    m_backend->bindPipeline(m_skyboxPipe);
    m_backend->setValue(m_skyboxPipe, "view", view_);
    m_backend->setValue(m_skyboxPipe, "proj", proj);
    m_backend->drawCubemap(m_cubemap);
}

void RenderSystem::renderMeshes(const std::vector<entt::entity>& entities, entt::entity cameraEntity, const glm::mat4& view, const glm::mat4& proj) noexcept
{
    auto [camera, cameraTransform] = m_registry.get<Camera, Transform>(cameraEntity);

    m_backend->bindPipeline(m_mainPipe);
    m_backend->setValue(m_mainPipe, "view", view);
    m_backend->setValue(m_mainPipe, "proj", proj);

    Frustum frustum { proj * view };

    glm::vec3 lightPos { -15.0f, 15.0f, 15.0f };
    m_backend->setValue(m_mainPipe, "light.position", lightPos);
    m_backend->setValue(m_mainPipe, "light.ambient", glm::vec3 { 0.2f, 0.2f, 0.2f });
    m_backend->setValue(m_mainPipe, "light.diffuse", glm::vec3 { 0.5f, 0.5f, 0.5f });
    m_backend->setValue(m_mainPipe, "light.specular", glm::vec3 { 1.0f, 1.0f, 1.0f });
    m_backend->setValue(m_mainPipe, "viewPos", cameraTransform.position);

    int drawed = 0;
    for (auto entity : entities) {
        auto [transform, renderer, bb] = m_registry.get<Transform, MeshRenderer, BoundingBox>(entity);

        auto aabb = toGlobalAABB(bb, transform);
        if (!frustum.isVisible(aabb))
            continue;
        drawed++;

        auto model = transform.getMatrix();
        m_backend->setValue(m_mainPipe, "model", model);
        m_backend->setValue(m_mainPipe, "material.ambient", glm::vec3 { 0.8f, 0.8f, 0.8f });
        m_backend->setValue(m_mainPipe, "material.diffuse", 0);
        m_backend->setValue(m_mainPipe, "material.specular", 1);
        m_backend->setValue(m_mainPipe, "material.shininess", 32.0f);

        m_backend->bindTexture(renderer.texture, 0);
        m_backend->bindTexture(renderer.specular, 1);

        auto& meshes = renderer.model->getMeshes();
        for (auto& mesh : meshes) {
            m_backend->drawMesh(mesh.meshID);
        }
    }
}

std::array<Line, 12> toLines(const BoundingBox& aabb, const Transform& transform) noexcept
{
    auto model = transform.getMatrix();

    glm::vec3 corners[] = {
        { aabb.min.x, aabb.min.y, aabb.min.z },
        { aabb.max.x, aabb.min.y, aabb.min.z },
        { aabb.min.x, aabb.max.y, aabb.min.z },
        { aabb.max.x, aabb.max.y, aabb.min.z },
        { aabb.min.x, aabb.min.y, aabb.max.z },
        { aabb.max.x, aabb.min.y, aabb.max.z },
        { aabb.min.x, aabb.max.y, aabb.max.z },
        { aabb.max.x, aabb.max.y, aabb.max.z }
    };

    std::array<Line, 12> result;
    result[0] = { corners[0], corners[1] };
    result[1] = { corners[1], corners[3] };
    result[2] = { corners[3], corners[2] };
    result[3] = { corners[2], corners[0] };
    result[4] = { corners[4], corners[5] };
    result[5] = { corners[5], corners[7] };
    result[6] = { corners[7], corners[6] };
    result[7] = { corners[6], corners[4] };
    result[8] = { corners[0], corners[4] };
    result[9] = { corners[1], corners[5] };
    result[10] = { corners[2], corners[6] };
    result[11] = { corners[3], corners[7] };

    for (auto& line : result) {
        line.p1 = glm::vec3 { model * glm::vec4 { line.p1, 1.0f } };
        line.p2 = glm::vec3 { model * glm::vec4 { line.p2, 1.0f } };
    }

    return result;
}

std::array<Line, 12> toLinesAligned(const BoundingBox& aabb, const Transform& transform) noexcept
{
    glm::vec3 corners[] = {
        { aabb.min.x, aabb.min.y, aabb.min.z },
        { aabb.max.x, aabb.min.y, aabb.min.z },
        { aabb.min.x, aabb.max.y, aabb.min.z },
        { aabb.max.x, aabb.max.y, aabb.min.z },
        { aabb.min.x, aabb.min.y, aabb.max.z },
        { aabb.max.x, aabb.min.y, aabb.max.z },
        { aabb.min.x, aabb.max.y, aabb.max.z },
        { aabb.max.x, aabb.max.y, aabb.max.z }
    };

    std::array<Line, 12> result;
    result[0] = { corners[0], corners[1] };
    result[1] = { corners[1], corners[3] };
    result[2] = { corners[3], corners[2] };
    result[3] = { corners[2], corners[0] };
    result[4] = { corners[4], corners[5] };
    result[5] = { corners[5], corners[7] };
    result[6] = { corners[7], corners[6] };
    result[7] = { corners[6], corners[4] };
    result[8] = { corners[0], corners[4] };
    result[9] = { corners[1], corners[5] };
    result[10] = { corners[2], corners[6] };
    result[11] = { corners[3], corners[7] };

    return result;
}