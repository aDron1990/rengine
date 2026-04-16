#pragma once

#include "components/Transform.hpp"
#include <cmath>
#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/euler_angles.hpp>

struct BoundingBox {
    glm::vec3 min { +INFINITY };
    glm::vec3 max { -INFINITY };
};

struct Picked { };

inline BoundingBox toGlobalAABB(const BoundingBox& aabb, const Transform& transform) noexcept
{
    auto model = transform.getMatrix();

    glm::vec3 corners[8] = {
        { aabb.min.x, aabb.min.y, aabb.min.z },
        { aabb.max.x, aabb.min.y, aabb.min.z },
        { aabb.min.x, aabb.max.y, aabb.min.z },
        { aabb.max.x, aabb.max.y, aabb.min.z },
        { aabb.min.x, aabb.min.y, aabb.max.z },
        { aabb.max.x, aabb.min.y, aabb.max.z },
        { aabb.min.x, aabb.max.y, aabb.max.z },
        { aabb.max.x, aabb.max.y, aabb.max.z }
    };

    glm::vec3 worldMin { +FLT_MAX };
    glm::vec3 worldMax { -FLT_MAX };

    for (int i = 0; i < 8; i++) {
        glm::vec3 transformed = glm::vec3(model * glm::vec4(corners[i], 1.0f));
        worldMin = glm::min(worldMin, transformed);
        worldMax = glm::max(worldMax, transformed);
    }

    return BoundingBox { worldMin, worldMax };
}

inline BoundingBox aligned(const BoundingBox& aabb, const Transform& transform) noexcept
{
    // auto model = transform.getMatrix();

    glm::mat4 model { 1.0f };
    model = glm::rotate(model, glm::radians(transform.rotation.x), { 1.0f, 0.0f, 0.0f });
    model = glm::rotate(model, glm::radians(transform.rotation.y), { 0.0f, 1.0f, 0.0f });
    model = glm::rotate(model, glm::radians(transform.rotation.z), { 0.0f, 0.0f, 1.0f });
    model = glm::scale(model, transform.scale);

    glm::vec3 corners[8] = {
        { aabb.min.x, aabb.min.y, aabb.min.z },
        { aabb.max.x, aabb.min.y, aabb.min.z },
        { aabb.min.x, aabb.max.y, aabb.min.z },
        { aabb.max.x, aabb.max.y, aabb.min.z },
        { aabb.min.x, aabb.min.y, aabb.max.z },
        { aabb.max.x, aabb.min.y, aabb.max.z },
        { aabb.min.x, aabb.max.y, aabb.max.z },
        { aabb.max.x, aabb.max.y, aabb.max.z }
    };

    glm::vec3 worldMin { +FLT_MAX };
    glm::vec3 worldMax { -FLT_MAX };

    for (int i = 0; i < 8; i++) {
        glm::vec3 transformed = glm::vec3(model * glm::vec4(corners[i], 1.0f));
        worldMin = glm::min(worldMin, transformed);
        worldMax = glm::max(worldMax, transformed);
    }

    return BoundingBox { worldMin, worldMax };
}
