#pragma once

#include "Mesh.hpp"
#include "Shader.hpp"
#include "Texture.hpp"
#include "components/Transform.hpp"
#include "utils.hpp"

#include <entt/entt.hpp>

class Object {
public:
    Object(entt::registry& registry, std::shared_ptr<Mesh> mesh, std::shared_ptr<Texture> texture);

    void draw(Shader& shader) const noexcept;

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

    std::shared_ptr<Mesh> m_mesh;
    std::shared_ptr<Texture> m_texture;

    glm::vec3 m_position { };
    glm::vec3 m_rotation { };
    glm::vec3 m_scale { 1.0f };
};
