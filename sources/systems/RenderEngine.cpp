#include "RenderEngine.hpp"
#include "BoundingBox.hpp"
#include "components/LineRenderer.hpp"
#include "components/MeshRenderer.hpp"
#include "components/SkyboxRenderer.hpp"
#include "components/Transform.hpp"
#include "graphics/RenderBackend.hpp"
#include "graphics/RenderContext.hpp"
#include "graphics/opengl/OglRenderBackend.hpp"
#include "graphics/vulkan/VulkanRenderBackend.hpp"
#include "utils/types.hpp"
#include "utils/utils.hpp"

#include <GLFW/glfw3.h>
#include <cassert>
#include <entt/entity/fwd.hpp>
#include <imgui.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/ext/matrix_transform.hpp>
#include <glm/geometric.hpp>
#include <glm/gtx/norm.hpp>

#include <array>
#include <memory>
#include <vector>

RenderEngine::RenderEngine(entt::registry& registry, uint32_t width, uint32_t height, GLFWwindow* window)
    : m_registry { registry }
    , m_size { width, height }
{
    //VulkanRenderBackend vulkan { window };
    m_backend.reset(new OglRenderBackend { { width, height } });
    m_registry.ctx().emplace<std::shared_ptr<RenderBackend>>(m_backend);
    auto& device = m_backend->getDevice();

    m_shadedMeshPipe = device.createPipeline({ "resources/shaders/shaded_mesh_v.glsl", "resources/shaders/shaded_mesh_f.glsl" }, RenderState { });
    m_meshPipe = device.createPipeline({ "resources/shaders/mesh_v.glsl", "resources/shaders/mesh_f.glsl" }, RenderState { });
    m_transparentPipe = device.createPipeline({ "resources/shaders/transparent_v.glsl", "resources/shaders/transparent_f.glsl" }, RenderState { });
    m_skyboxPipe = device.createPipeline({ "resources/shaders/cubemap_v.glsl", "resources/shaders/cubemap_f.glsl" }, RenderState { .depth = false });
    m_linesPipe = device.createPipeline({ "resources/shaders/line_v.glsl", "resources/shaders/line_f.glsl" }, RenderState { .depth = false });

    addPass(std::make_unique<SkyboxPass>(m_skyboxPipe));
    addPass(std::make_unique<MeshPass>(m_shadedMeshPipe, m_meshPipe));
    addPass(std::make_unique<LinePass>(m_linesPipe));

    auto cubeImages = loadCubeImages("resources/images/space_skybox");
    m_cubemap = device.createCubemap(cubeImages);
    m_registry.emplace<SkyboxRenderer>(m_registry.create()).cubemap = m_cubemap;
}

void RenderEngine::resize(uint32_t width, uint32_t height) noexcept
{
    m_backend->getDevice().resizeDefaultFramebuffer(width, height);
    m_size = { width, height };
}

int RenderEngine::addRenderLayer(uint32_t width, uint32_t height, entt::entity camera) noexcept
{
    auto texture = m_backend->getDevice().createRenderTexture(width, height);
    m_layers.emplace_back(RenderLayer { texture, camera });
    return m_layers.size() - 1;
}

void RenderEngine::setRenderLayerCamera(int nlayer, entt::entity camera) noexcept
{
    if (nlayer == DEFAULT_RENDER_LAYER) {
        m_defaultLayerCamera = camera;
        return;
    }
    assert(m_layers.size() > nlayer);
    m_layers[nlayer].camera = camera;
}

ImTextureID RenderEngine::getGuiTextureFromLayer(int nlayer) noexcept
{
    assert(m_layers.size() > nlayer && nlayer != DEFAULT_RENDER_LAYER);
    return (ImTextureID)m_backend->getDevice().getGuiTexture(m_layers[nlayer].texture);
}

void RenderEngine::addPass(std::unique_ptr<RenderPass> pass) noexcept
{
    m_passes.emplace_back(std::move(pass));
}

void RenderEngine::render() noexcept
{
    RenderContext ctx {
        .registry = m_registry,
        .layers = m_layers,
        .defaultLayerCamera = m_defaultLayerCamera,
        .defaultLayerSize = m_size,
    };

    for (auto& pass : m_passes) {
        pass->collect(ctx);
        pass->render(*m_backend, ctx);
    }

    m_backend->getCommandBuffer().bindDefaultFramebuffer();
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
