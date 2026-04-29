#pragma once

#include <entt/entity/fwd.hpp>
#include <entt/entt.hpp>
#include <glm/ext/vector_double3.hpp>
#include <glm/ext/vector_float3.hpp>

class OriginRebaseSystem {
public:
    static constexpr double SimulationRadiusKm = 3.0;
    static constexpr double RebaseStepKm = 3.0;
    static constexpr double MetersPerKilometer = 1000.0;

    explicit OriginRebaseSystem(entt::registry& registry);

    void update() noexcept;
    void syncTransforms() noexcept;
    void syncTransform(entt::entity entity) noexcept;

    [[nodiscard]] glm::dvec3 getOriginKm() const noexcept { return m_originKm; }
    void setOriginKm(const glm::dvec3& originKm) noexcept;

    [[nodiscard]] glm::vec3 toLocalMeters(const glm::dvec3& positionKm) const noexcept;
    [[nodiscard]] glm::dvec3 toWorldKm(const glm::vec3& positionMeters) const noexcept;

private:
    void rebaseAroundAnchor() noexcept;
    [[nodiscard]] glm::dvec3 snappedOriginFor(const glm::dvec3& positionKm) const noexcept;

private:
    entt::registry& m_registry;
    glm::dvec3 m_originKm { };
};
