#include "Object.hpp"

#include <glm/gtc/matrix_transform.hpp>

Object::Object(std::shared_ptr<Mesh> mesh, std::shared_ptr<Texture> texture) :
	m_mesh{ mesh },
	m_texture{ texture } {}

void Object::draw(Shader& shader) const noexcept {
	auto model = glm::mat4{ 1.0f };
	model = glm::translate(model, m_position);
	model = glm::rotate(model, glm::radians(m_rotation.x), { 1.0f, 0.0f, 0.0f });
	model = glm::rotate(model, glm::radians(m_rotation.y), { 0.0f, 1.0f, 0.0f });
	model = glm::rotate(model, glm::radians(m_rotation.z), { 0.0f, 0.0f, 1.0f });
	model = glm::scale(model, m_scale);

	shader.setUniform(model, "model");
	m_texture->bind();
	m_mesh->draw();
}

glm::vec3& Object::position() noexcept {
	return m_position;
}

glm::vec3& Object::rotation() noexcept {
	return m_rotation;
}

glm::vec3& Object::scale() noexcept {
	return m_scale;
}

const glm::vec3& Object::position() const noexcept {
	return m_position;
}

const glm::vec3& Object::rotation() const noexcept {
	return m_rotation;
}

const glm::vec3& Object::scale() const noexcept {
	return m_scale;
}
