#include "OrbitalEngine.hpp"
#include "Clock.hpp"
#include "components/Celestial.hpp"
#include "components/OrbitalBody.hpp"
#include "components/Transform.hpp"
#include "systems/PhysicsEngine.hpp"
#include <glm/geometric.hpp>
#include <iostream>
#include <ostream>

OrbiralEngine::OrbiralEngine(entt::registry& registry)
    : m_registry { registry }
{
}

void OrbiralEngine::update() noexcept
{
    auto& physics = m_registry.ctx().get<PhysicsEngine>();
    auto dt = 1.0f / 60.0f;
    auto celView = m_registry.view<Celestial, Transform>();

    auto [celestial, celTransform] = m_registry.get<Celestial, Transform>(celView.front());
    for (auto [entity, body, transform] : m_registry.view<OrbitalBody, Transform>().each()) {
        auto acc = [center = celTransform.position, GM = celestial.GM](const glm::vec3& pos) {
            auto r = center - pos;
            auto dist = glm::length(r);
            return glm::normalize(r) * (GM / (dist * dist));
        };

        body.velicity += 0.5f * acc(transform.position) * dt;
        transform.position += body.velicity * dt;
        body.velicity += 0.5f * acc(transform.position) * dt;

        physics.applyTransform(entity);
    }
}