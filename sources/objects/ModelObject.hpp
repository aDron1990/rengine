#pragma once

#include "Model.hpp"
#include "Object.hpp"

#include "Texture.hpp"

class ModelObject : public Object {
public:
    ModelObject(entt::registry& registry, std::shared_ptr<Model> model, std::shared_ptr<Texture> texture, std::shared_ptr<Texture> specular);

    glm::vec3& position() noexcept;
    glm::quat& rotation() noexcept;
    glm::vec3& scale() noexcept;

    const glm::vec3& position() const noexcept;
    const glm::quat& rotation() const noexcept;
    const glm::vec3& scale() const noexcept;
};