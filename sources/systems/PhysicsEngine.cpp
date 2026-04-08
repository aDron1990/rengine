#include "PhysicsEngine.hpp"
#include "BoundingBox.hpp"
#include "components/Body.hpp"
#include "components/Transform.hpp"

#include <Jolt/Core/Factory.h>
#include <Jolt/Core/Memory.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Geometry/Plane.h>
#include <Jolt/Math/Real.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/PlaneShape.h>
#include <Jolt/Physics/PhysicsSettings.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/RegisterTypes.h>
#include <glm/ext/vector_float4.hpp>
#include <glm/trigonometric.hpp>

PhysicsEngine::PhysicsEngine(entt::registry& registry, JPH::TempAllocatorImpl& tempAllocator, JPH::JobSystemThreadPool& jobSystem)
    : m_registry { registry }
    , m_tempAllocator { tempAllocator }
    , m_jobSystem { jobSystem }
{
    m_world.Init(1024,
        0,
        1024,
        1024,
        m_broad_phase_layer_interface,
        m_object_vs_bp_filter,
        m_object_layer_pair_filter);

    m_registry.storage<Body>();
    m_registry.on_destroy<Body>().connect<&PhysicsEngine::destroyBody>(this);
}

void PhysicsEngine::update() noexcept
{
    m_world.Update(1.0f / 180.0f, 1, &m_tempAllocator, &m_jobSystem);
    auto view = m_registry.view<Body, Transform>();
    for (auto [entity, body, transform] : view.each()) {
        JPH::RVec3 position;
        JPH::Quat rotation;
        auto& ibody = m_world.GetBodyInterface();
        ibody.GetPositionAndRotation(body.bodyID, position, rotation);
        auto rot = rotation.GetEulerAngles();
        transform.position = { position.GetX(), position.GetY(), position.GetZ() };
        transform.rotation = { glm::degrees(rot.GetX()), glm::degrees(rot.GetY()), glm::degrees(rot.GetZ()) };
    }
}

void PhysicsEngine::createCollider(entt::entity entity, bool dynamic)
{
    if (!m_registry.all_of<BoundingBox, Transform>(entity))
        return;

    auto [bb_, transform] = m_registry.get<BoundingBox, Transform>(entity);
    auto bb = bb_;
    auto model = glm::mat4 { 1.0f };
    model = glm::scale(model, transform.scale);
    bb.min = model * glm::vec4 { bb.min, 1.0f };
    bb.max = model * glm::vec4 { bb.max, 1.0f };

    auto size = JPH::Vec3 { (bb.max.x - bb.min.x) / 2.0f, (bb.max.y - bb.min.y) / 2.0f, (bb.max.z - bb.min.z) / 2.0f };
    auto position = JPH::Vec3 { transform.position.x, transform.position.y, transform.position.z };
    glm::quat quat = transform.getQuat();
    auto rotation = JPH::Quat { quat.x, quat.y, quat.z, quat.w };

    JPH::BoxShapeSettings box_shape(size);
    JPH::BodyCreationSettings box_settings(
        box_shape.Create().Get(),
        position,
        rotation,
        dynamic ? JPH::EMotionType::Dynamic : JPH::EMotionType::Static,
        dynamic ? Layers::MOVING : Layers::NON_MOVING);

    JPH::Body* box = m_world.GetBodyInterface().CreateBody(box_settings);
    m_world.GetBodyInterface().AddBody(box->GetID(), dynamic ? JPH::EActivation::Activate : JPH::EActivation::DontActivate);
    m_registry.emplace_or_replace<Body>(entity, Body { box->GetID() });
}

void PhysicsEngine::destroyBody(entt::registry& registry, entt::entity entity)
{
    auto& ibody = m_world.GetBodyInterface();
    auto& body = m_registry.get<Body>(entity);
    ibody.RemoveBody(body.bodyID);
    ibody.DestroyBody(body.bodyID);
}

void PhysicsEngine::applyTransform(entt::entity entity) noexcept
{
    if (!m_registry.all_of<Body, Transform>(entity))
        return;

    auto [body, transform] = m_registry.get<Body, Transform>(entity);
    JPH::RVec3 position = { transform.position.x, transform.position.y, transform.position.z };
    JPH::RVec3 rotation = { glm::radians(transform.rotation.x), glm::radians(transform.rotation.y), glm::radians(transform.rotation.z) };
    auto quat = JPH::Quat::sEulerAngles(rotation);
    auto& ibody = m_world.GetBodyInterface();
    ibody.SetPositionAndRotation(body.bodyID, position, quat, JPH::EActivation::Activate);
}