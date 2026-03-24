#pragma once

#include <glm/glm.hpp>

#include <chrono>

class Camera {
public:
    enum class Direction { Front,
        Back,
        Left,
        Right,
        Up,
        Down };

    Camera(const glm::vec3& position);

    void move(Direction dir) noexcept;
    void rotate(float yaw, float pitch) noexcept;
    void update() noexcept;
    const glm::mat4& getView() const noexcept;
    const glm::vec3& getPos() const noexcept;

private:
    glm::vec3 m_position;
    glm::vec3 m_front;
    glm::vec3 m_up;
    glm::mat4 m_view { 1.0f };

    std::chrono::steady_clock::time_point m_lastUpdateTime {
        std::chrono::steady_clock::now()
    };
    float m_deltaTime { 0.0f };
    const float m_speed { 2.5f };
};
