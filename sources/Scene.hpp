#pragma once

#include "Object.hpp"

#include <entt/entt.hpp>
#include <memory>
#include <type_traits>
#include <utility>
#include <vector>

class Scene {
public:
    Scene() = default;
    ~Scene() = default;

    Scene(const Scene&) = delete;
    Scene& operator=(const Scene&) = delete;
    Scene(Scene&&) = delete;
    Scene& operator=(Scene&&) = delete;

    [[nodiscard]] entt::registry& registry() noexcept { return m_registry; }
    [[nodiscard]] const entt::registry& registry() const noexcept { return m_registry; }

    [[nodiscard]] entt::entity createEntity() { return m_registry.create(); }
    void destroyEntity(entt::entity entity);

    template <typename T, typename... Args>
        requires std::is_base_of_v<Object, T>
    T& createObject(Args&&... args)
    {
        auto object = std::make_unique<T>(m_registry, std::forward<Args>(args)...);
        auto& result = *object;
        m_objects.push_back(std::move(object));
        return result;
    }

    template <typename Func>
    void forEachObject(Func&& func)
    {
        for (auto& object : m_objects) {
            func(*object);
        }
    }

    template <typename Func>
    void forEachObject(Func&& func) const
    {
        for (const auto& object : m_objects) {
            func(*object);
        }
    }

    void update();
    void clear();

private:
    entt::registry m_registry;
    std::vector<std::unique_ptr<Object>> m_objects;
};
