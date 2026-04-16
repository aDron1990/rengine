#pragma once

#include "LineBatch.hpp"
#include <entt/entity/fwd.hpp>
#include <entt/entt.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/glm.hpp>

class OrbiralEngine {
public:
    OrbiralEngine(entt::registry& registry);
    void update() noexcept;

    struct Step {
        glm::vec3 position;
        glm::vec3 velocity;
    };
    Step orbitalStep(const glm::vec3& center, const glm::vec3& position, const glm::vec3& velocity, float GM) const noexcept;
    LineBatch calcOrbit(entt::entity object, entt::entity center) const noexcept;
    struct OrbitParams {
        glm::vec3 center;
        glm::vec3 ex; // major axis direction
        glm::vec3 ey; // minor axis direction
        float a; // semi-major axis
        float e; // eccentricity
    };
    OrbitParams computeOrbit(const glm::vec3& r, const glm::vec3& v, float GM) const noexcept;

private:
    entt::registry& m_registry;
    const float DT = 1.0f / 120.0f;
};