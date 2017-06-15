#include "Physics/PhysicsComponent.h"
#include "Physics/PhysicsModule.h"
#include "Physics/PhysicsActor.h"

#include <Engine/Engine.h>
#include <Engine/EngineContext.h>

#include <ModuleManager/ModuleManager.h>
#include <Reflection/ReflectionRegistrator.h>

namespace DAVA
{
uint32 PhysicsComponent::GetType() const
{
    return Component::PHYSICS_COMPONENT;
}

DAVA::Component* PhysicsComponent::Clone(Entity* toEntity)
{
    PhysicsComponent* result = new PhysicsComponent();
    result->SetEntity(toEntity);

    Physics* physics = GetEngineContext()->moduleManager->GetModule<Physics>();
    result->actor = physics->CloneActor(actor, result);

    return result;
}

void PhysicsComponent::Serialize(KeyedArchive* archive, SerializationContext* serializationContext)
{
    Component::Serialize(archive, serializationContext);
}

void PhysicsComponent::Deserialize(KeyedArchive* archive, SerializationContext* serializationContext)
{
    Component::Deserialize(archive, serializationContext);
}

DAVA_VIRTUAL_REFLECTION_IMPL(PhysicsComponent)
{
    ReflectionRegistrator<PhysicsComponent>::Begin()
    .ConstructorByPointer()
    .End();
}

} // namespace DAVA
