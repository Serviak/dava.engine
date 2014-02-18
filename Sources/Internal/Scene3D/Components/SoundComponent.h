/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/



#ifndef __DAVAENGINE_SCENE3D_SOUND_COMPONENT_H__
#define __DAVAENGINE_SCENE3D_SOUND_COMPONENT_H__

#include "Base/BaseTypes.h"
#include "Entity/Component.h"
#include "Scene3D/Entity.h"
#include "Scene3D/SceneFile/SerializationContext.h"
#include "Sound/SoundEvent.h"

namespace DAVA 
{

class SoundComponent : public Component
{
public:
    SoundComponent();
    virtual ~SoundComponent();

    virtual Component * Clone(Entity * toEntity);

    virtual void Serialize(KeyedArchive *archive, SerializationContext *serializationContext);
    virtual void Deserialize(KeyedArchive *archive, SerializationContext *serializationContext);
    
    inline uint32 GetEventsCount() const;
    inline SoundEvent * GetSoundEvent(uint32 index) const;

    void AddSoundEvent(SoundEvent * event);
    void RemoveSoundEvent(SoundEvent * event);
    void RemoveAllEvents();
    
    IMPLEMENT_COMPONENT_TYPE(SOUND_COMPONENT);
    
protected:
    Vector<SoundEvent *> events;

public:
    INTROSPECTION_EXTEND(SoundComponent, Component,
        COLLECTION(events, "Events Array",  I_VIEW)
        );
};

//Inline
inline SoundEvent * SoundComponent::GetSoundEvent(uint32 index) const
{
    DVASSERT(index >= 0 && index < (uint32)events.size());
    return events[index];
}

inline uint32 SoundComponent::GetEventsCount() const
{
    return events.size();
}

};

#endif