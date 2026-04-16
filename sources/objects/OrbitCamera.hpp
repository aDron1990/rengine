#pragma once

#include "Object.hpp"
#include <entt/entity/fwd.hpp>

class OrbitCamera : public Object {
public:
    OrbitCamera(entt::registry& registry, entt::entity target);
    void update() noexcept;
    void rotate(float yaw, float pitch) noexcept;

private:
    float m_yaw = -90.0f;
    float m_pitch = 0.0f;
    float m_distance = 10.0f;
    entt::entity m_target;
};