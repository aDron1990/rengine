#include "OrbitCamera.hpp"
#include "Input.hpp"
#include "Object.hpp"
#include "components/Camera.hpp"
#include "components/Transform.hpp"
#include <glm/geometric.hpp>
#include <glm/trigonometric.hpp>

OrbitCamera::OrbitCamera(entt::registry& registry, entt::entity target)
    : Object(registry)
    , m_target { target }
{
    addComponent(Camera { });
}

void OrbitCamera::update() noexcept
{
    auto& input = m_registry.ctx().get<Input>();
    if (input.getButton(GLFW_MOUSE_BUTTON_MIDDLE) || input.getButton(GLFW_MOUSE_BUTTON_RIGHT))
        if (auto delta = input.getCursorDelta(); delta) {
            float sensitivity = 0.3f;
            auto xoffset = delta->x * sensitivity;
            auto yoffset = -delta->y * sensitivity;

            m_yaw += xoffset;
            m_pitch += yoffset;

            if (m_pitch > 89.0f)
                m_pitch = 89.0f;
            if (m_pitch < -89.0f)
                m_pitch = -89.0f;
        }

    auto scroll = input.getScrollDelta();
    if (scroll) {
        m_distance -= scroll->y;
    }

    glm::vec3 offset;
    offset.x = m_distance * cos(glm::radians(m_pitch)) * sin(glm::radians(m_yaw));
    offset.y = m_distance * sin(glm::radians(m_pitch));
    offset.z = m_distance * cos(glm::radians(m_pitch)) * cos(glm::radians(m_yaw));

    auto target = m_registry.get<Transform>(m_target);
    auto& transform = getComponent<Transform>();
    auto& camera = getComponent<Camera>();
    transform.position = target.position + offset;
    camera.front = glm::normalize(target.position - transform.position);
}

void OrbitCamera::rotate(float yaw, float pitch) noexcept
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
