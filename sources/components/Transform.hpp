#pragma once

#include <glm/ext/vector_float3.hpp>
#include <glm/glm.hpp>

struct Transform {
    glm::vec3 position { };
    glm::vec3 rotation { };
    glm::vec3 scale { 1.0f };

    glm::mat4 getMatrix() const noexcept;
    glm::vec3 transformPoint(const glm::vec3& point) const noexcept;
    glm::quat getQuat() const noexcept;
};
