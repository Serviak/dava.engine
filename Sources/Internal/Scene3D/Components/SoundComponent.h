/*==================================================================================
    Copyright (c) 2008, DAVA, INC
    All rights reserved.

    Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the DAVA, INC nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE DAVA CONSULTING, LLC AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL DAVA CONSULTING, LLC BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/

#ifndef __DAVAENGINE_SCENE3D_SOUND_COMPONENT_H__
#define __DAVAENGINE_SCENE3D_SOUND_COMPONENT_H__

#include "Base/BaseTypes.h"
#include "Entity/Component.h"
#include "Scene3D/Entity.h"

namespace DAVA 
{

class SoundEvent;
class SoundUpdateSystem;
class SoundComponent : public Component
{
public:
	SoundComponent();
	virtual ~SoundComponent();

	IMPLEMENT_COMPONENT_TYPE(SOUND_COMPONENT);

	SoundEvent * GetSoundEvent();
	const String & GetEventName();
	void SetEventName(const String & eventName);

	virtual Component * Clone(Entity * toEntity);
	virtual void Serialize(KeyedArchive *archive, SceneFileV2 *sceneFile);
	virtual void Deserialize(KeyedArchive *archive, SceneFileV2 *sceneFile);

private:
	void SetSoundEvent(SoundEvent * sEvent);

	SoundEvent * soundEvent;
	String eventName;

	friend class SoundUpdateSystem;

public:
	INTROSPECTION_EXTEND(SoundComponent, Component,
		PROPERTY("eventName", "eventName", GetEventName, SetEventName, INTROSPECTION_SERIALIZABLE | INTROSPECTION_EDITOR)
		);
};

};

#endif