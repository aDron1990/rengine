#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include "Scene.hpp"
#include "components/OriginAnchor.hpp"
#include "components/Transform.hpp"
#include "components/WorldPosition.hpp"
#include "systems/OriginRebaseSystem.hpp"

#include <entt/entt.hpp>

class TestObject : public Object {
public:
    explicit TestObject(entt::registry& registry)
        : Object { registry }
    {
    }

    void update() override { ++updates; }

    int updates = 0;
};

TEST_CASE("doctest test runner is wired")
{
    CHECK(1 + 1 == 2);
}

TEST_CASE("scene owns registry and updates created objects")
{
    Scene scene;

    auto entity = scene.createEntity();
    scene.registry().emplace<WorldPosition>(entity, WorldPosition { { 1.0, 2.0, 3.0 } });

    auto& first = scene.createObject<TestObject>();
    auto& second = scene.createObject<TestObject>();

    scene.update();
    scene.update();

    CHECK(scene.registry().valid(entity));
    CHECK(first.hasComponent<Transform>());
    CHECK(first.updates == 2);
    CHECK(second.updates == 2);
}

TEST_CASE("origin rebase converts world kilometers to local meters")
{
    entt::registry registry;
    OriginRebaseSystem origin { registry };

    auto entity = registry.create();
    registry.emplace<WorldPosition>(entity, WorldPosition { { 2.0, 0.0, 0.0 } });
    registry.emplace<Transform>(entity);

    origin.syncTransforms();
    CHECK(registry.get<Transform>(entity).position.x == doctest::Approx(2000.0f));

    origin.setOriginKm({ 3.0, 0.0, 0.0 });
    CHECK(registry.get<Transform>(entity).position.x == doctest::Approx(-1000.0f));
}

TEST_CASE("origin rebase follows anchor in three kilometer steps")
{
    entt::registry registry;
    OriginRebaseSystem origin { registry };

    auto anchor = registry.create();
    registry.emplace<OriginAnchor>(anchor);
    registry.emplace<WorldPosition>(anchor, WorldPosition { { 3.1, 0.0, 0.0 } });
    registry.emplace<Transform>(anchor);

    bool rebased = origin.update();

    CHECK(rebased);
    CHECK(origin.getOriginKm().x == doctest::Approx(3.0));
    CHECK(registry.get<Transform>(anchor).position.x == doctest::Approx(100.0f));
}
