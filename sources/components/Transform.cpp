#include "Transform.hpp"
#include <glm/geometric.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

glm::mat4 Transform::getMatrix() const noexcept
{
    auto model = glm::mat4 { 1.0f };
    model = glm::translate(model, position);
    model *= glm::toMat4( glm::normalize(rotation));
    model = glm::scale(model, scale);
    return model;
}

glm::vec3 Transform::getEulerAngles() const noexcept
{
    return glm::eulerAngles(rotation);
}

glm::vec3 Transform::transformPoint(const glm::vec3& point) const noexcept
{
    return getMatrix() * glm::vec4 { point, 1.0f };
}