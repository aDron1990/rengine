#include "FlyingCamera.hpp"
#include "Input.hpp"
#include "Object.hpp"
#include "components/Camera.hpp"
#include "components/Transform.hpp"
#include "systems/Clock.hpp"

FlyingCamera::FlyingCamera(entt::registry& registry, glm::vec3 position)
    : Object(registry)
{
    addComponent(Camera { });
    getComponent<Transform>().position = position;
}

void FlyingCamera::update()
{
    auto& input = m_registry.ctx().get<Input>();
    auto [camera, transform] = m_registry.get<Camera, Transform>(m_entity);

    if (input.getKey(GLFW_KEY_W))
        move(Direction::Front);
    if (input.getKey(GLFW_KEY_S))
        move(Direction::Back);
    if (input.getKey(GLFW_KEY_A))
        move(Direction::Left);
    if (input.getKey(GLFW_KEY_D))
        move(Direction::Right);
    if (input.getKey(GLFW_KEY_SPACE))
        move(Direction::Up);
    if (input.getKey(GLFW_KEY_LEFT_SHIFT))
        move(Direction::Down);

    if (input.getButton(GLFW_MOUSE_BUTTON_MIDDLE))
        if (auto delta = input.getCursorDelta(); delta) {
            float sensitivity = 0.1f;
            auto xoffset = -delta->x * sensitivity;
            auto yoffset = delta->y * sensitivity;

            m_yaw += xoffset;
            m_pitch += yoffset;

            rotate(m_yaw, m_pitch);
        }
}

void FlyingCamera::move(Direction dir) noexcept
{
    auto delta = m_registry.ctx().get<Clock>().getDelta();
    auto [camera, transform] = m_registry.get<Camera, Transform>(m_entity);
    switch (dir) {
    case Direction::Front:
        transform.position += glm::normalize(glm::vec3 { camera.front.x, 0.0f, camera.front.z }) * SPEED * delta;
        break;
    case Direction::Back:
        transform.position -= glm::normalize(glm::vec3 { camera.front.x, 0.0f, camera.front.z }) * SPEED * delta;
        break;
    case Direction::Left:
        transform.position -= glm::normalize(glm::cross(camera.front, camera.up)) * SPEED * delta;
        break;
    case Direction::Right:
        transform.position += glm::normalize(glm::cross(camera.front, camera.up)) * SPEED * delta;
        break;
    case Direction::Up:
        transform.position += camera.up * SPEED * delta;
        break;
    case Direction::Down:
        transform.position -= camera.up * SPEED * delta;
        break;
    }
}

void FlyingCamera::rotate(float yaw, float pitch) noexcept
{
    auto [camera, transform] = m_registry.get<Camera, Transform>(m_entity);
    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;

    glm::vec3 direction;
    direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    direction.y = sin(glm::radians(pitch));
    direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    camera.front = glm::normalize(direction);
}
