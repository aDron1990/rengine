#pragma once

#include <glm/glm.hpp>

struct Camera {
    glm::vec3 front { 0.0f, 0.0, -1.0f };
    glm::vec3 up { 0.0f, 1.0f, 0.0f };
    glm::mat4 getView(const glm::vec3& position) const noexcept;
    glm::mat4 getProj(float aspect) const noexcept;
    float near = .1f;
    float far = 1000.f;
    float fov = 60.0f;
};