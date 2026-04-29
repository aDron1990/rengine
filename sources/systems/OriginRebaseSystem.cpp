#include "OriginRebaseSystem.hpp"
#include "components/OriginAnchor.hpp"
#include "components/Transform.hpp"
#include "components/WorldPosition.hpp"

#include <cmath>

OriginRebaseSystem::OriginRebaseSystem(entt::registry& registry)
    : m_registry { registry }
{
}

bool OriginRebaseSystem::update() noexcept
{
    bool rebased = rebaseAroundAnchor();
    syncTransforms();
    return rebased;
}

void OriginRebaseSystem::syncTransforms() noexcept
{
    for (auto&& [entity, worldPosition, transform] : m_registry.view<WorldPosition, Transform>().each()) {
        transform.position = toLocalMeters(worldPosition.positionKm);
    }
}

void OriginRebaseSystem::syncTransform(entt::entity entity) noexcept
{
    if (!m_registry.all_of<WorldPosition, Transform>(entity))
        return;

    auto& worldPosition = m_registry.get<WorldPosition>(entity);
    auto& transform = m_registry.get<Transform>(entity);
    transform.position = toLocalMeters(worldPosition.positionKm);
}

void OriginRebaseSystem::setOriginKm(const glm::dvec3& originKm) noexcept
{
    m_originKm = originKm;
    syncTransforms();
}

glm::vec3 OriginRebaseSystem::toLocalMeters(const glm::dvec3& positionKm) const noexcept
{
    return glm::vec3((positionKm - m_originKm) * MetersPerKilometer);
}

glm::dvec3 OriginRebaseSystem::toWorldKm(const glm::vec3& positionMeters) const noexcept
{
    return m_originKm + glm::dvec3(positionMeters) / MetersPerKilometer;
}

bool OriginRebaseSystem::rebaseAroundAnchor() noexcept
{
    auto view = m_registry.view<OriginAnchor, WorldPosition>();
    if (view.begin() == view.end())
        return false;

    auto entity = view.front();
    auto& worldPosition = m_registry.get<WorldPosition>(entity);
    auto localKm = worldPosition.positionKm - m_originKm;
    if (std::abs(localKm.x) < SimulationRadiusKm
        && std::abs(localKm.y) < SimulationRadiusKm
        && std::abs(localKm.z) < SimulationRadiusKm) {
        return false;
    }

    m_originKm = snappedOriginFor(worldPosition.positionKm);
    return true;
}

glm::dvec3 OriginRebaseSystem::snappedOriginFor(const glm::dvec3& positionKm) const noexcept
{
    return glm::dvec3 {
        std::trunc(positionKm.x / RebaseStepKm) * RebaseStepKm,
        std::trunc(positionKm.y / RebaseStepKm) * RebaseStepKm,
        std::trunc(positionKm.z / RebaseStepKm) * RebaseStepKm,
    };
}
