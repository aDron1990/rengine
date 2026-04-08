#include "Transform.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

glm::mat4 Transform::getMatrix() const noexcept
{
    auto model = glm::mat4 { 1.0f };
    model = glm::translate(model, position);
    model *= glm::toMat4(getQuat());
    model = glm::scale(model, scale);
    return model;
}

glm::quat Transform::getQuat() const noexcept
{
    return glm::angleAxis(glm::radians(rotation.z), glm::vec3(0, 0, 1)) * glm::angleAxis(glm::radians(rotation.y), glm::vec3(0, 1, 0)) * glm::angleAxis(glm::radians(rotation.x), glm::vec3(1, 0, 0));
}

glm::vec3 Transform::transformPoint(const glm::vec3& point) const noexcept
{
    return getMatrix() * glm::vec4 { point, 1.0f };
}