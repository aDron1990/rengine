#include "Camera.hpp"

#include <glm/gtc/matrix_transform.hpp>

glm::mat4 Camera::getView(const glm::vec3& position) const noexcept
{
    return glm::lookAt(position, position + front, up);
}

glm::mat4 Camera::getProj(float aspect) const noexcept
{
    if (type == ProjectionType::Perspective)
        return glm::perspective(glm::radians(fov), aspect, near, far);

    float halfHeight = orthoSize;
    float halfWidth = orthoSize * aspect;
    return glm::ortho(-halfWidth, halfWidth, -halfHeight, halfHeight, near, far);
}
