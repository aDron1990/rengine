#include "Scene.hpp"

void Scene::destroyEntity(entt::entity entity)
{
    if (m_registry.valid(entity)) {
        m_registry.destroy(entity);
    }
}

void Scene::update()
{
    for (auto& object : m_objects) {
        object->update();
    }
}

void Scene::clear()
{
    m_objects.clear();
    m_registry.clear();
}
