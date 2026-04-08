#pragma once

#include <Jolt/Jolt.h>

#include <Jolt/Core/Factory.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Physics/Body/BodyID.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <entt/entity/fwd.hpp>
#include <entt/entt.hpp>

namespace Layers {
static constexpr JPH::ObjectLayer NON_MOVING = 0;
static constexpr JPH::ObjectLayer MOVING = 1;
static constexpr JPH::uint NUM_LAYERS = 2;
}

class BPLayerInterfaceImpl final : public JPH::BroadPhaseLayerInterface {
public:
    JPH::uint GetNumBroadPhaseLayers() const override
    {
        return 2;
    }

    JPH::BroadPhaseLayer GetBroadPhaseLayer(JPH::ObjectLayer layer) const override
    {
        switch (layer) {
        case Layers::NON_MOVING:
            return JPH::BroadPhaseLayer(0);
        case Layers::MOVING:
            return JPH::BroadPhaseLayer(1);
        default:
            return JPH::BroadPhaseLayer(0);
        }
    }
};

class ObjectVsBroadPhaseLayerFilterImpl : public JPH::ObjectVsBroadPhaseLayerFilter {
public:
    bool ShouldCollide(JPH::ObjectLayer obj, JPH::BroadPhaseLayer bp) const override
    {
        switch (obj) {
        case Layers::NON_MOVING:
            return bp == JPH::BroadPhaseLayer(1);

        case Layers::MOVING:
            return true;

        default:
            return false;
        }
    }
};

class ObjectLayerPairFilterImpl : public JPH::ObjectLayerPairFilter {
public:
    bool ShouldCollide(JPH::ObjectLayer a, JPH::ObjectLayer b) const override
    {
        if (a == Layers::NON_MOVING && b == Layers::NON_MOVING)
            return false;

        return true;
    }
};

class PhysicsEngine {
public:
    PhysicsEngine(entt::registry& registry, JPH::TempAllocatorImpl& tempAllocator, JPH::JobSystemThreadPool& jobSystem);
    void update() noexcept;

    void applyTransform(entt::entity entity) noexcept;
    void createCollider(entt::entity entity, bool dynamic = true);

private:
    void destroyBody(entt::registry& registry, entt::entity entity);

private:
    JPH::TempAllocatorImpl& m_tempAllocator;
    JPH::JobSystemThreadPool& m_jobSystem;
    BPLayerInterfaceImpl m_broad_phase_layer_interface;
    ObjectVsBroadPhaseLayerFilterImpl m_object_vs_bp_filter;
    ObjectLayerPairFilterImpl m_object_layer_pair_filter;
    JPH::PhysicsSystem m_world;
    entt::registry& m_registry;
};