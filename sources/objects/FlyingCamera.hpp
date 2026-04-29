#pragma once

#include "Object.hpp"

#include <glm/glm.hpp>

class FlyingCamera : public Object {
public:
    FlyingCamera(entt::registry& registry, glm::vec3 position);
    void update() override;

    enum class Direction { Front,
        Back,
        Left,
        Right,
        Up,
        Down };
    void move(Direction dir) noexcept;
    void rotate(float yaw, float pitch) noexcept;

private:
    float m_yaw = -90.0f;
    float m_pitch = 0.0f;
    const float SPEED = 5.5f;
};
