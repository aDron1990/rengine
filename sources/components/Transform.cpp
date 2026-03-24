#include "Transform.hpp"

#include <glm/gtc/matrix_transform.hpp>

glm::mat4 Transform::getMatrix() const noexcept
{
    auto model = glm::mat4 { 1.0f };
    model = glm::translate(model, position);
    model = glm::rotate(model, glm::radians(rotation.x), { 1.0f, 0.0f, 0.0f });
    model = glm::rotate(model, glm::radians(rotation.y), { 0.0f, 1.0f, 0.0f });
    model = glm::rotate(model, glm::radians(rotation.z), { 0.0f, 0.0f, 1.0f });
    model = glm::scale(model, scale);
    return model;
}
