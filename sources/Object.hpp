#pragma once

#include "Mesh.hpp"
#include "Shader.hpp"
#include "Texture.hpp"
#include "components/Renderer.hpp"
#include "components/Transform.hpp"
#include "utils.hpp"

#include <entt/entt.hpp>

class Object {
public:
    Object(entt::registry& registry, std::shared_ptr<Mesh> mesh, std::shared_ptr<Texture> texture, std::shared_ptr<Texture> specular);

    void draw(Shader& shader) const noexcept;

    template <typename T>
    void addComponent(const T& component) noexcept
    {
        m_registry.get_or_emplace<T>(m_entity, component);
    }

    glm::vec3& position() noexcept;
    glm::vec3& rotation() noexcept;
    glm::vec3& scale() noexcept;

    const glm::vec3& position() const noexcept;
    const glm::vec3& rotation() const noexcept;
    const glm::vec3& scale() const noexcept;

private:
    entt::registry& m_registry;
    entt::entity m_entity;

    Transform& m_transform;
    Renderer& m_renderer;
};
