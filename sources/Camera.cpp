#include "Camera.hpp"

#include <glm/gtc/matrix_transform.hpp>

Camera::Camera(const glm::vec3& position) : 
	m_position{ position },
	m_front{ 0.0f, 0.0, -1.0f },
	m_up{ 0.0f, 1.0f, 0.0f } { }

void Camera::move(Direction dir) noexcept {
	switch (dir) {
	case Direction::Front:
		m_position += glm::normalize(glm::vec3{ m_front.x, 0.0f, m_front.z }) * m_deltaTime * m_speed; break;
	case Direction::Back:
		m_position -= glm::normalize(glm::vec3{ m_front.x, 0.0f, m_front.z }) * m_deltaTime * m_speed; break;
	case Direction::Left:
		m_position -= glm::normalize(glm::cross(m_front, m_up)) * m_deltaTime * m_speed; break;
	case Direction::Right:
		m_position += glm::normalize(glm::cross(m_front, m_up)) * m_deltaTime * m_speed; break;
	case Direction::Up:
		m_position += m_up * m_deltaTime * m_speed; break;
	case Direction::Down:
		m_position -= m_up * m_deltaTime * m_speed; break;
	}
}

void Camera::rotate(float yaw, float pitch) noexcept {
	if (pitch > 89.0f)
		pitch = 89.0f;
	if (pitch < -89.0f)
		pitch = -89.0f;

	glm::vec3 direction;
	direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	direction.y = sin(glm::radians(pitch));
	direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	m_front = glm::normalize(direction); 
}

void Camera::update() noexcept {
	auto now = std::chrono::steady_clock::now();
	m_deltaTime = std::chrono::duration_cast<std::chrono::milliseconds>(now - m_lastUpdateTime).count() / 1000.0f;
	m_lastUpdateTime = now;

	m_view = glm::lookAt(m_position, m_position + m_front, m_up);
}

const glm::mat4& Camera::getView() const noexcept {
	return m_view;
}
