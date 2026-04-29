#include "PhysicsEngine.hpp"
#include "BoundingBox.hpp"
#include "Clock.hpp"
#include "components/Body.hpp"
#include "components/Transform.hpp"

#include <Jolt/Core/Core.h>
#include <Jolt/Core/Factory.h>
#include <Jolt/Core/Memory.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Geometry/Plane.h>
#include <Jolt/Math/Real.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Collision/CastResult.h>
#include <Jolt/Physics/Collision/RayCast.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/PlaneShape.h>
#include <Jolt/Physics/Collision/Shape/RotatedTranslatedShape.h>
#include <Jolt/Physics/PhysicsSettings.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/RegisterTypes.h>

#include <glm/ext/vector_float3.hpp>
#include <glm/ext/vector_float4.hpp>
#include <glm/geometric.hpp>
#include <glm/trigonometric.hpp>

#include <optional>

PhysicsEngine::PhysicsEngine(entt::registry& registry, JPH::TempAllocatorImpl& tempAllocator, JPH::JobSystemThreadPool& jobSystem)
    : m_registry { registry }
    , m_tempAllocator { tempAllocator }
    , m_jobSystem { jobSystem }
{
    m_world.Init(1024,
        0,
        4096,
        4096,
        m_broad_phase_layer_interface,
        m_object_vs_bp_filter,
        m_object_layer_pair_filter);

    m_world.SetGravity({ 0, 0, 0 });
    m_registry.storage<Body>();
    m_registry.on_destroy<Body>().connect<&PhysicsEngine::destroyBody>(this);
}

void PhysicsEngine::update() noexcept
{
    m_world.Update(m_registry.ctx().get<Clock>().getDelta(), 1, &m_tempAllocator, &m_jobSystem);
    auto view = m_registry.view<Body, Transform>();
    for (auto&& [entity, body, transform] : view.each()) {
        JPH::RVec3 position;
        JPH::Quat rotation;
        auto& ibody = m_world.GetBodyInterface();
        ibody.GetPositionAndRotation(body.bodyID, position, rotation);
        transform.position = { position.GetX(), position.GetY(), position.GetZ() };
        transform.rotation = { rotation.GetXYZW().GetW(), rotation.GetXYZW().GetX(), rotation.GetXYZW().GetY(), rotation.GetXYZW().GetZ() };
    }
}

void PhysicsEngine::createCollider(entt::entity entity, bool dynamic)
{
    if (!m_registry.all_of<BoundingBox, Transform>(entity))
        return;

    auto bb = m_registry.get<BoundingBox>(entity);
    auto& transform = m_registry.get<Transform>(entity);
    auto model = glm::mat4 { 1.0f };
    model = glm::scale(model, transform.scale);

    bb.min = model * glm::vec4 { bb.min, 1.0f };
    bb.max = model * glm::vec4 { bb.max, 1.0f };

    auto size = JPH::Vec3 { (bb.max.x - bb.min.x) / 2.0f, (bb.max.y - bb.min.y) / 2.0f, (bb.max.z - bb.min.z) / 2.0f };
    auto position = JPH::Vec3 { transform.position.x, transform.position.y, transform.position.z };
    glm::quat quat = transform.rotation;
    auto rotation = JPH::Quat { quat.x, quat.y, quat.z, quat.w };

    glm::vec3 offset = (bb.min + bb.max) * 0.5f;

    JPH::Ref<JPH::BoxShape> box_shape = new JPH::BoxShape(
        size);

    JPH::Ref<JPH::Shape> shape = new JPH::RotatedTranslatedShape(
        JPH::Vec3(offset.x, offset.y, offset.z),
        JPH::Quat::sIdentity(),
        box_shape);

    JPH::BodyCreationSettings box_settings(
        shape,
        position,
        rotation,
        dynamic ? JPH::EMotionType::Dynamic : JPH::EMotionType::Static,
        dynamic ? Layers::MOVING : Layers::NON_MOVING);

    JPH::Body* box = m_world.GetBodyInterface().CreateBody(box_settings);
    box->SetUserData(static_cast<JPH::uint64>(entity));
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

    auto& body = m_registry.get<Body>(entity);
    auto& transform = m_registry.get<Transform>(entity);
    JPH::RVec3 position = { transform.position.x, transform.position.y, transform.position.z };
    JPH::RVec3 rotation = { glm::radians(transform.rotation.x), glm::radians(transform.rotation.y), glm::radians(transform.rotation.z) };
    auto quat = JPH::Quat::sEulerAngles(rotation);
    auto& ibody = m_world.GetBodyInterface();
    ibody.SetPositionAndRotation(body.bodyID, position, quat, JPH::EActivation::Activate);
}

std::optional<entt::entity> PhysicsEngine::pick(const Ray& ray) const noexcept
{
    JPH::Vec3 origin = { ray.origin.x, ray.origin.y, ray.origin.z };
    JPH::Vec3 direction = { ray.direction.x, ray.direction.y, ray.direction.z };

    JPH::RRayCast raycast(origin, direction * 5000.0f);

    JPH::RayCastResult hit;
    if (!m_world.GetNarrowPhaseQuery().CastRay(raycast, hit))
        return std::nullopt;

    auto& ibody = m_world.GetBodyInterface();
    return static_cast<entt::entity>(ibody.GetUserData(hit.mBodyID));
}

void PhysicsEngine::addImpulse(entt::entity entity, glm::vec3 impulse) noexcept
{
    if (!m_registry.all_of<Body>(entity))
        return;

    auto [bodyID] = m_registry.get<Body>(entity);
    JPH::RVec3 impulse_ = { impulse.x, impulse.y, impulse.z };
    auto& ibody = m_world.GetBodyInterface();
    ibody.AddImpulse(bodyID, impulse_);
}

void PhysicsEngine::addForce(entt::entity entity, glm::vec3 force) noexcept
{
    if (!m_registry.all_of<Body>(entity))
        return;

    auto [bodyID] = m_registry.get<Body>(entity);
    JPH::RVec3 force_ = { force.x, force.y, force.z };
    auto& ibody = m_world.GetBodyInterface();
    ibody.AddForce(bodyID, force_);
}

glm::vec3 PhysicsEngine::getVelocity(entt::entity entity) const noexcept
{
    if (!m_registry.all_of<Body>(entity))
        return glm::vec3 { 0.0f };

    auto [bodyID] = m_registry.get<Body>(entity);
    auto& ibody = m_world.GetBodyInterface();
    auto velocity = ibody.GetLinearVelocity(bodyID);
    return { velocity.GetX(), velocity.GetY(), velocity.GetZ() };
}
