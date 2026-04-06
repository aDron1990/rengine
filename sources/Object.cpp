#include "Object.hpp"

#include <glm/gtc/matrix_transform.hpp>

Object::Object(entt::registry& registry, std::shared_ptr<Mesh> mesh, std::shared_ptr<Texture> texture, std::shared_ptr<Texture> specular)
    : m_registry { registry }
    , m_entity { m_registry.create() }
    , m_transform { m_registry.emplace<Transform>(m_entity) }
    , m_renderer { m_registry.emplace<Renderer>(m_entity, Renderer { mesh, texture, specular }) }
    , m_aabb { m_registry.emplace<BoundingBox>(m_entity, mesh->getAABB()) }
{
}

void Object::draw(Shader& shader) const noexcept
{
    auto model = m_transform.getMatrix();

    shader.setUniform(model, "model");
    m_renderer.texture->bind();
    m_renderer.mesh->draw();
}

glm::vec3& Object::position() noexcept { return m_transform.position; }

glm::vec3& Object::rotation() noexcept { return m_transform.rotation; }

glm::vec3& Object::scale() noexcept { return m_transform.scale; }

const glm::vec3& Object::position() const noexcept { return m_transform.position; }

const glm::vec3& Object::rotation() const noexcept { return m_transform.rotation; }

const glm::vec3& Object::scale() const noexcept { return m_transform.scale; }
