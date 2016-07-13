#ifndef __PARTICLE_EMITTER_MOVE_COMMANDS_H__
#define __PARTICLE_EMITTER_MOVE_COMMANDS_H__

#include "Commands2/Base/RECommand.h"
#include "Scene3D/Components/ParticleEffectComponent.h"
#include "Particles/ParticleEmitter.h"

class ParticleEmitterMoveCommand : public RECommand
{
public:
    ParticleEmitterMoveCommand(DAVA::ParticleEffectComponent* oldEffect, DAVA::ParticleEmitterInstance* emitter, DAVA::ParticleEffectComponent* newEffect, int newIndex);

    void Undo() override;
    void Redo() override;
    DAVA::Entity* GetEntity() const override;

private:
    DAVA::RefPtr<DAVA::ParticleEmitterInstance> instance;
    DAVA::ParticleEffectComponent* oldEffect;
    DAVA::ParticleEffectComponent* newEffect;
    DAVA::int32 oldIndex = -1;
    DAVA::int32 newIndex;
};

inline DAVA::Entity* ParticleEmitterMoveCommand::GetEntity() const
{
    return nullptr;
}

#endif