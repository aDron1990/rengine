#include "TestSatelite.hpp"
#include "Input.hpp"
#include "ModelObject.hpp"
#include "components/Celestial.hpp"
#include "components/LineRenderer.hpp"
#include "components/OrbitalBody.hpp"
#include "systems/Clock.hpp"
#include "systems/OrbitalEngine.hpp"
#include "systems/PhysicsEngine.hpp"
#include <GLFW/glfw3.h>

TestSatelite::TestSatelite(entt::registry& regisrty, std::shared_ptr<Model> model, TextureID texture, TextureID specular)
    : ModelObject(regisrty, model, texture, specular)
{
    addComponent(OrbitalBody { { 0.0f, 0.0f, 4.0f } });
    addComponent(NavballSourceTag { });
    addComponent(LineRenderer { });
    auto& orbitalEngine = m_registry.ctx().get<OrbiralEngine>();
    getComponent<LineRenderer>().lines = orbitalEngine.calcOrbit(getEntity(), m_registry.view<Celestial>().front());
    auto& physics = m_registry.ctx().get<PhysicsEngine>();
    physics.createCollider(getEntity(), true);
}

void TestSatelite::update() noexcept
{
    auto& input = m_registry.ctx().get<Input>();
    auto delta = m_registry.ctx().get<Clock>().getDelta();
    auto& transform = getComponent<Transform>();
    auto& body = getComponent<OrbitalBody>();

    float yaw { };
    float pitch { };
    float roll { };
    const float angle = 1 * delta;
    bool accelerate = false;
    if (input.getKey(GLFW_KEY_W))
        pitch -= angle;
    if (input.getKey(GLFW_KEY_S))
        pitch += angle;

    if (input.getKey(GLFW_KEY_D))
        yaw -= angle;
    if (input.getKey(GLFW_KEY_A))
        yaw += angle;

    if (input.getKey(GLFW_KEY_E))
        roll -= angle;
    if (input.getKey(GLFW_KEY_Q))
        roll += angle;

    if (input.getKey(GLFW_KEY_SPACE))
        accelerate = true;

    glm::quat qPitch = glm::angleAxis(pitch, glm::vec3(1, 0, 0));
    glm::quat qYaw = glm::angleAxis(yaw, glm::vec3(0, 1, 0));
    glm::quat qRoll = glm::angleAxis(roll, glm::vec3(0, 0, 1));
    transform.rotation = transform.rotation * qPitch * qYaw * qRoll;

    if (!accelerate)
        return;

    auto front = (transform.rotation * glm::vec3(0, 0, -1));
    body.velocity += front * 1.f * delta;

    auto& orbitalEngine = m_registry.ctx().get<OrbiralEngine>();
    auto& renderer = getComponent<LineRenderer>();
    renderer.lines = orbitalEngine.calcOrbit(getEntity(), m_registry.view<Celestial>().front());
}