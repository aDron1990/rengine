#pragma once

#include <glm/glm.hpp>

struct Transform {
    glm::vec3 position { };
    glm::vec3 rotation { };
    glm::vec3 scale { 1.0f };

    glm::mat4 getMatrix() const noexcept;
};
