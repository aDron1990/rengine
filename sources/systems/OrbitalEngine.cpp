#include "OrbitalEngine.hpp"
#include "components/Celestial.hpp"
#include "components/OrbitalBody.hpp"
#include "components/Transform.hpp"
#include "components/WorldPosition.hpp"
#include "systems/OriginRebaseSystem.hpp"
#include "systems/PhysicsEngine.hpp"
#include <glm/geometric.hpp>
#include <glm/trigonometric.hpp>

OrbiralEngine::OrbiralEngine(entt::registry& registry)
    : m_registry { registry }
{
}

void OrbiralEngine::update() noexcept
{
    auto& physics = m_registry.ctx().get<PhysicsEngine>();
    auto& originRebase = m_registry.ctx().get<OriginRebaseSystem>();

    auto celView = m_registry.view<Celestial, WorldPosition>();
    auto& celestial = m_registry.get<Celestial>(celView.front());
    auto& celPosition = m_registry.get<WorldPosition>(celView.front());
    for (auto&& [entity, body, worldPosition] : m_registry.view<OrbitalBody, WorldPosition>().each()) {
        auto step = orbitalStep(celPosition.positionKm, worldPosition.positionKm, body.velocityKmPerSec, celestial.GM);
        worldPosition.positionKm = step.positionKm;
        body.velocityKmPerSec = step.velocityKmPerSec;
        originRebase.syncTransform(entity);
        physics.applyTransform(entity);
    }
}

OrbiralEngine::Step OrbiralEngine::orbitalStep(const glm::dvec3& centerKm,
    const glm::dvec3& positionKm,
    const glm::dvec3& velocityKmPerSec,
    double GM) const noexcept
{
    auto acc = [centerKm, GM](const glm::dvec3& pos) {
        auto r = centerKm - pos;
        auto dist = glm::length(r);
        return glm::normalize(r) * (GM / (dist * dist));
    };

    Step step { positionKm, velocityKmPerSec };
    step.velocityKmPerSec += 0.5 * acc(step.positionKm) * DT;
    step.positionKm += step.velocityKmPerSec * DT;
    step.velocityKmPerSec += 0.5 * acc(step.positionKm) * DT;

    return step;
}

std::vector<Line> OrbiralEngine::calcOrbit(entt::entity object, entt::entity center) const noexcept
{
    std::vector<Line> result;
    result.reserve(256);

    auto& objBody = m_registry.get<OrbitalBody>(object);
    auto& objPosition = m_registry.get<WorldPosition>(object);
    auto& celBody = m_registry.get<Celestial>(center);
    auto& celPosition = m_registry.get<WorldPosition>(center);
    auto& originRebase = m_registry.ctx().get<OriginRebaseSystem>();

    OrbitParams orb = computeOrbit(
        objPosition.positionKm - celPosition.positionKm,
        objBody.velocityKmPerSec,
        celBody.GM);

    const int segments = 256;
    glm::vec3 prev;
    bool first = true;
    for (int i = 0; i < segments; i++) {
        double nu = (double)i / segments * glm::two_pi<double>();
        double r = orb.a * (1.0 - orb.e * orb.e) / (1.0 + orb.e * std::cos(nu));

        glm::dvec3 worldPosKm = celPosition.positionKm + orb.ex * (r * std::cos(nu)) + orb.ey * (r * std::sin(nu));
        glm::vec3 pos = originRebase.toLocalMeters(worldPosKm);
        if (!first) {
            result.push_back({ prev, pos });
        } else {
            first = false;
        }

        prev = pos;
    }

    return result;
}

OrbiralEngine::OrbitParams OrbiralEngine::computeOrbit(const glm::dvec3& rKm,
    const glm::dvec3& velocityKmPerSec,
    double GM) const noexcept
{
    double mu = GM;
    glm::dvec3 h = glm::cross(rKm, velocityKmPerSec);
    glm::dvec3 eVec = (glm::cross(velocityKmPerSec, h) / mu) - glm::normalize(rKm);

    double e = glm::length(eVec);
    double rLen = glm::length(rKm);
    double v2 = glm::dot(velocityKmPerSec, velocityKmPerSec);
    double a = 1.0 / (2.0 / rLen - v2 / mu);

    glm::dvec3 ex = glm::normalize(eVec);
    glm::dvec3 ey = glm::normalize(glm::cross(h, ex));

    return { { }, ex, ey, a, e };
}
