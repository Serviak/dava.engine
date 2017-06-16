#pragma once

#include "Physics/PhysicsComponent.h"

#include <Reflection/Reflection.h>

namespace DAVA
{
class StaticBodyComponent : public PhysicsComponent
{
public:
    uint32 GetType() const override;
    Component* Clone(Entity* toEntity) override;

    void Serialize(KeyedArchive* archive, SerializationContext* serializationContext) override;
    void Deserialize(KeyedArchive* archive, SerializationContext* serializationContext) override;

protected:
#if defined(__DAVAENGINE_DEBUG__)
    void CheckActorType() const override;
#endif

private:
    DAVA_VIRTUAL_REFLECTION(StaticBodyComponent, PhysicsComponent);
};
} // namespace DAVA