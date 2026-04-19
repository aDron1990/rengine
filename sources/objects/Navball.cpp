#include "Navball.hpp"
#include "ModelObject.hpp"
#include "TestSatelite.hpp"
#include "components/Camera.hpp"
#include "components/Celestial.hpp"
#include "components/LineRenderer.hpp"
#include "components/MeshRenderer.hpp"
#include "components/Transform.hpp"
#include "systems/RenderSystem.hpp"
#include <entt/entity/fwd.hpp>
#include <glm/ext/matrix_projection.hpp>
#include <glm/ext/quaternion_float.hpp>
#include <glm/ext/quaternion_geometric.hpp>
#include <glm/ext/quaternion_transform.hpp>
#include <glm/geometric.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/matrix.hpp>

Navball::Navball(entt::registry& registry, std::shared_ptr<Model> model, TextureID texture)
    : ModelObject(registry, model, texture, texture)
{
    addComponent<LineRenderer>(LineRenderer{});

    m_debugLines = registry.create();
    registry.emplace<LineRenderer>(m_debugLines);

    m_cameraEntity = registry.create();
    auto& transform = getComponent<Transform>();
    auto cameraPos = (transform.rotation * glm::vec3 { 0, 0, 4.0f });

    registry.emplace<Camera>(m_cameraEntity, Camera { });
    registry.get<Camera>(m_cameraEntity).fov = 30.0f;
    registry.get<Camera>(m_cameraEntity).front = glm::normalize(-cameraPos);
    registry.emplace<Transform>(m_cameraEntity, Transform { .position = cameraPos });

    m_renderlayer = registry.ctx().get<std::reference_wrapper<RenderSystem>>().get().addRenderLayer(400, 400, m_cameraEntity);
    getComponent<MeshRenderer>().layer = m_renderlayer;
    getComponent<LineRenderer>().layer = m_renderlayer;
}

void Navball::update() noexcept
{
    auto& transform = getComponent<Transform>();
    auto target = m_registry.view<NavballSourceTag>().front();
    auto& targetTrans = m_registry.get<Transform>(target);
    auto celEntity = m_registry.view<Celestial>().front();
    auto& celTrans = m_registry.get<Transform>(celEntity);
    auto [camera, camTrans] = m_registry.get<Camera, Transform>(m_cameraEntity);

    glm::vec3 surfaceUp = glm::normalize(targetTrans.position - celTrans.position);
    glm::vec3 worldNorth = glm::vec3(0, 1, 0);
    if (glm::abs(glm::dot(surfaceUp, worldNorth)) > 0.999f)
        worldNorth = glm::vec3(0, 0, 1);
    glm::vec3 east = glm::normalize(glm::cross(worldNorth, surfaceUp));
    glm::vec3 north = glm::cross(surfaceUp, east);

    // glm::mat3 surfaceBasis = glm::mat3(east, surfaceUp, north);
    glm::quat surfaceRotation = glm::quatLookAt(north, surfaceUp);
    glm::quat relativeRot = glm::inverse(surfaceRotation) * targetTrans.rotation;

    auto fix_pitch = glm::angleAxis(glm::pi<float>(), glm::vec3(1, 0, 0));
    auto fix_yaw = glm::angleAxis(glm::pi<float>(), glm::vec3(0, 1, 0));
    auto align_fix = glm::angleAxis(glm::half_pi<float>(), glm::vec3(0, -1, 0));

    transform.rotation = fix_pitch * fix_yaw * glm::inverse(relativeRot) * align_fix * -fix_pitch;
    // transform.rotation = glm::inverse(relativeRot);

    auto& renderer = m_registry.get<LineRenderer>(m_debugLines);
    renderer.lines = {
        { targetTrans.position, targetTrans.position + north },
        { targetTrans.position, targetTrans.position + east },
        { targetTrans.position, targetTrans.position + surfaceUp },
        { targetTrans.position, celTrans.position },
    };
    renderer.color = {1.0f, 1.0f, 1.0f, 0.5f};
}