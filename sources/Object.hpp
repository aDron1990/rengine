#pragma once

#include "utils.hpp"
#include "Mesh.hpp"
#include "Texture.hpp"
#include "Shader.hpp"

class Object {
public:
	Object(std::shared_ptr<Mesh> mesh, std::shared_ptr<Texture> texture);

	void draw(Shader& shader) const noexcept;

	glm::vec3& position() noexcept;
	glm::vec3& rotation() noexcept;
	glm::vec3& scale() noexcept;

	const glm::vec3& position() const noexcept;
	const glm::vec3& rotation() const noexcept;
	const glm::vec3& scale() const noexcept;

private:
	std::shared_ptr<Mesh> m_mesh;
	std::shared_ptr<Texture> m_texture;
	
	glm::vec3 m_position{};
	glm::vec3 m_rotation{};
	glm::vec3 m_scale{ 1.0f };

};

