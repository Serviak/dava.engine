#include "Physics/PhysicsComponent.h"
#include "Physics/PhysicsModule.h"

#include <Reflection/ReflectionRegistrator.h>

#include <physx/PxActor.h>

namespace DAVA
{
void PhysicsComponent::Serialize(KeyedArchive* archive, SerializationContext* serializationContext)
{
    Component::Serialize(archive, serializationContext);
}

void PhysicsComponent::Deserialize(KeyedArchive* archive, SerializationContext* serializationContext)
{
    Component::Deserialize(archive, serializationContext);
}

physx::PxActor* PhysicsComponent::GetPxActor() const
{
    return actor;
}

void PhysicsComponent::SetPxActor(physx::PxActor* actor_)
{
    DVASSERT(actor_ != nullptr);
    DVASSERT(actor == nullptr);
    actor = actor_;
    actor->userData = this;
#if defined(__DAVAENGINE_DEBUG__)
    ValidateActorType();
#endif
}

void PhysicsComponent::ReleasePxActor()
{
    DVASSERT(actor != nullptr);
    actor->release();
    actor = nullptr;
}

DAVA_VIRTUAL_REFLECTION_IMPL(PhysicsComponent)
{
    ReflectionRegistrator<PhysicsComponent>::Begin()
    .End();
}

} // namespace DAVA
