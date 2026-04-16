#include "OrbitalEngine.hpp"
#include "LineBatch.hpp"
#include "components/Celestial.hpp"
#include "components/OrbitalBody.hpp"
#include "components/Transform.hpp"
#include "systems/PhysicsEngine.hpp"
#include <glm/geometric.hpp>

OrbiralEngine::OrbiralEngine(entt::registry& registry)
    : m_registry { registry }
{
}

void OrbiralEngine::update() noexcept
{
    auto& physics = m_registry.ctx().get<PhysicsEngine>();

    auto celView = m_registry.view<Celestial, Transform>();
    auto [celestial, celTransform] = m_registry.get<Celestial, Transform>(celView.front());
    for (auto [entity, body, transform] : m_registry.view<OrbitalBody, Transform>().each()) {
        auto step = orbitalStep(celTransform.position, transform.position, body.velocity, celestial.GM);
        transform.position = step.position;
        body.velocity = step.velocity;
        physics.applyTransform(entity);
    }
}

OrbiralEngine::Step OrbiralEngine::orbitalStep(const glm::vec3& center, const glm::vec3& position, const glm::vec3& velocity, float GM) const noexcept
{
    auto acc = [center, GM](const glm::vec3& pos) {
        auto r = center - pos;
        auto dist = glm::length(r);
        return glm::normalize(r) * (GM / (dist * dist));
    };

    Step step { position, velocity };
    step.velocity += 0.5f * acc(step.position) * DT;
    step.position += step.velocity * DT;
    step.velocity += 0.5f * acc(step.position) * DT;

    return step;
}

LineBatch OrbiralEngine::calcOrbit(entt::entity object, entt::entity center) const noexcept
{
    LineBatch result;

    auto [objBody, objTrans] = m_registry.get<OrbitalBody, Transform>(object);
    auto [celBody, celTrans] = m_registry.get<Celestial, Transform>(center);

    OrbitParams orb = computeOrbit(
        objTrans.position - celTrans.position,
        objBody.velocity,
        celBody.GM);

    const int segments = 256;
    glm::vec3 prev;
    bool first = true;
    for (int i = 0; i < segments; i++) {
        float nu = (float)i / segments * glm::two_pi<float>();
        float r = orb.a * (1.0f - orb.e * orb.e) / (1.0f + orb.e * std::cos(nu));

        glm::vec3 pos = celTrans.position + orb.ex * (r * std::cos(nu)) + orb.ey * (r * std::sin(nu));
        if (!first) {
            result.push({ prev, pos });
        } else {
            first = false;
        }

        prev = pos;
    }

    return result;
}

OrbiralEngine::OrbitParams OrbiralEngine::computeOrbit(const glm::vec3& r,
    const glm::vec3& v,
    float GM) const noexcept
{
    float mu = GM;
    glm::vec3 h = glm::cross(r, v);
    glm::vec3 eVec = (glm::cross(v, h) / mu) - glm::normalize(r);

    float e = glm::length(eVec);
    float rLen = glm::length(r);
    float v2 = glm::dot(v, v);
    float a = 1.0f / (2.0f / rLen - v2 / mu);

    glm::vec3 ex = glm::normalize(eVec);
    glm::vec3 ey = glm::normalize(glm::cross(h, ex));

    return { { }, ex, ey, a, e };
}