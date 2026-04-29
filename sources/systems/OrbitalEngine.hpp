#pragma once

#include "utils/types.hpp"
#include <entt/entity/fwd.hpp>
#include <entt/entt.hpp>
#include <glm/ext/vector_double3.hpp>
#include <glm/glm.hpp>
#include <vector>

class OrbiralEngine {
public:
    OrbiralEngine(entt::registry& registry);
    void update() noexcept;

    struct Step {
        glm::dvec3 positionKm;
        glm::dvec3 velocityKmPerSec;
    };
    Step orbitalStep(const glm::dvec3& centerKm, const glm::dvec3& positionKm, const glm::dvec3& velocityKmPerSec, double GM) const noexcept;
    std::vector<Line> calcOrbit(entt::entity object, entt::entity center) const noexcept;
    struct OrbitParams {
        glm::dvec3 centerKm;
        glm::dvec3 ex; // major axis direction
        glm::dvec3 ey; // minor axis direction
        double a; // semi-major axis, km
        double e; // eccentricity
    };
    OrbitParams computeOrbit(const glm::dvec3& rKm, const glm::dvec3& velocityKmPerSec, double GM) const noexcept;

private:
    entt::registry& m_registry;
    const double DT = 1.0 / 120.0;
};
