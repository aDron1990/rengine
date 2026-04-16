#include "Object.hpp"
#include "components/Transform.hpp"

#include <glm/gtc/matrix_transform.hpp>

Object::Object(entt::registry& registry)
    : m_registry { registry }
    , m_entity { m_registry.create() }
{
    m_registry.emplace<Transform>(m_entity);
}
