#include "EditorPhysics/Private/EditorPhysicsSystem.h"

#include <Physics/PhysicsSystem.h>
#include <Physics/DynamicBodyComponent.h>

#include <Scene3D/Entity.h>
#include <Scene3D/Scene.h>
#include <Entity/Component.h>

#include <physx/PxRigidDynamic.h>

EditorPhysicsSystem::EditorPhysicsSystem(DAVA::Scene* scene)
    : SceneSystem(scene)
{
    scene->physicsSystem->SetDebugDrawEnabled(true);
    scene->physicsSystem->SetSimulationEnabled(false);
}

void EditorPhysicsSystem::RegisterEntity(DAVA::Entity* entity)
{
    if (entity->GetComponentCount(DAVA::Component::DYNAMIC_BODY_COMPONENT) > 0 ||
        entity->GetComponentCount(DAVA::Component::STATIC_BODY_COMPONENT) > 0)
    {
        EntityInfo& info = transformMap[entity];
        info.originalTransform = entity->GetWorldTransform();
        info.isLocked = entity->GetLocked();
    }
}

void EditorPhysicsSystem::UnregisterEntity(DAVA::Entity* entity)
{
    auto iter = transformMap.find(entity);
    if (iter != transformMap.end())
    {
        transformMap.erase(iter);
    }
}

void EditorPhysicsSystem::RegisterComponent(DAVA::Entity* entity, DAVA::Component* component)
{
    DAVA::uint32 componentType = component->GetType();
    if (componentType == DAVA::Component::STATIC_BODY_COMPONENT ||
        componentType == DAVA::Component::DYNAMIC_BODY_COMPONENT)
    {
        RegisterEntity(entity);
    }
}

void EditorPhysicsSystem::UnregisterComponent(DAVA::Entity* entity, DAVA::Component* component)
{
    DAVA::uint32 componentType = component->GetType();
    if (componentType == DAVA::Component::STATIC_BODY_COMPONENT ||
        componentType == DAVA::Component::DYNAMIC_BODY_COMPONENT)
    {
        DAVA::uint32 actorCount = entity->GetComponentCount(DAVA::Component::STATIC_BODY_COMPONENT);
        actorCount += entity->GetComponentCount(DAVA::Component::DYNAMIC_BODY_COMPONENT);
        if (actorCount == 1)
        {
            UnregisterEntity(entity);
        }
    }
}

void EditorPhysicsSystem::Process(DAVA::float32 timeElapsed)
{
    if (state != eSimulationState::STOPPED)
    {
        for (auto& node : transformMap)
        {
            node.first->SetLocked(true);
        }
    }
}

void EditorPhysicsSystem::SetSimulationState(eSimulationState newState)
{
    if (newState == state)
    {
        return;
    }

    DAVA::PhysicsSystem* physicsSystem = GetScene()->physicsSystem;

    switch (newState)
    {
    case eSimulationState::PLAYING:
        DVASSERT(physicsSystem->IsSimulationEnabled() == false);
        if (state == eSimulationState::STOPPED)
        {
            StoreActualTransform();
        }
        physicsSystem->SetSimulationEnabled(true);
        break;
    case eSimulationState::PAUSED:
        DVASSERT(physicsSystem->IsSimulationEnabled() == true);
        DVASSERT(state == eSimulationState::PLAYING);
        physicsSystem->SetSimulationEnabled(false);
        break;
    case eSimulationState::STOPPED:
        DVASSERT(physicsSystem->IsSimulationEnabled() == true || state == eSimulationState::PAUSED);
        physicsSystem->SetSimulationEnabled(false);
        RestoreTransform();
        break;
    default:
        break;
    }

    state = newState;
}

EditorPhysicsSystem::eSimulationState EditorPhysicsSystem::GetSimulationState() const
{
    return state;
}

void EditorPhysicsSystem::StoreActualTransform()
{
    for (auto& node : transformMap)
    {
        node.second.originalTransform = node.first->GetWorldTransform();
        node.second.isLocked = node.first->GetLocked();
    }
}

void EditorPhysicsSystem::RestoreTransform()
{
    for (auto& node : transformMap)
    {
        node.first->SetWorldTransform(node.second.originalTransform);
        node.first->SetLocked(node.second.isLocked);

        DAVA::PhysicsComponent* component = static_cast<DAVA::PhysicsComponent*>(node.first->GetComponent(DAVA::Component::DYNAMIC_BODY_COMPONENT, 0));
        if (component != nullptr)
        {
            physx::PxRigidDynamic* actor = component->GetPxActor()->is<physx::PxRigidDynamic>();
            if (actor != nullptr)
            {
                actor->setLinearVelocity(physx::PxVec3(0.0f, 0.0f, 0.0f));
                actor->setAngularVelocity(physx::PxVec3(0.0f, 0.0f, 0.0f));
            }
        }
    }
}
