#pragma once

#include <entt/entt.hpp>
class OrbiralEngine {
public:
    OrbiralEngine(entt::registry& registry);
    void update() noexcept;

private:
    entt::registry& m_registry;
};