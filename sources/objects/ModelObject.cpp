#include "ModelObject.hpp"
#include "Model.hpp"
#include "Object.hpp"

#include "components/Renderer.hpp"
#include "components/Transform.hpp"

ModelObject::ModelObject(entt::registry& registry, std::shared_ptr<Model> model, std::shared_ptr<Texture> texture, std::shared_ptr<Texture> specular)
    : Object(registry)
{
    addComponent(Renderer { model, texture, specular });
    addComponent(model->getAABB());
}

glm::vec3& ModelObject::position() noexcept { return getComponent<Transform>().position; }
glm::quat& ModelObject::rotation() noexcept { return getComponent<Transform>().rotation; }
glm::vec3& ModelObject::scale() noexcept { return getComponent<Transform>().scale; }

const glm::vec3& ModelObject::position() const noexcept { return getComponent<Transform>().position; }
const glm::quat& ModelObject::rotation() const noexcept { return getComponent<Transform>().rotation; }
const glm::vec3& ModelObject::scale() const noexcept { return getComponent<Transform>().scale; }