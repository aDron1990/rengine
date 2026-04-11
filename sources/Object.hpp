#pragma once

#include <entt/entt.hpp>

class Object {
public:
    Object(entt::registry& registry);
    virtual ~Object() { m_registry.destroy(m_entity); };

    template <typename T>
    void addComponent(const T& component) noexcept
    {
        m_registry.get_or_emplace<T>(m_entity, component);
    }

    template <typename T>
    bool hasComponent() noexcept
    {
        return m_registry.all_of<T>(m_entity);
    }

    template <typename T>
    T& getComponent() noexcept
    {
        return m_registry.get<T>(m_entity);
    }

    template <typename T>
    const T& getComponent() const noexcept
    {
        return m_registry.get<T>(m_entity);
    }

    entt::entity getEntity() const noexcept { return m_entity; }

protected:
    entt::registry& m_registry;
    entt::entity m_entity;
};
