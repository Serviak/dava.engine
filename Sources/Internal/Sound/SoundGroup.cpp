/*==================================================================================
    Copyright (c) 2008, DAVA Consulting, LLC
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the DAVA Consulting, LLC nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE DAVA CONSULTING, LLC AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL DAVA CONSULTING, LLC BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

    Revision History:
        * Created by Igor Solovey
=====================================================================================*/

#include "Sound/SoundGroup.h"
#include "Sound/Sound.h"
#include "Animation/LinearAnimation.h"
#include "Sound/FMODUtils.h"

namespace DAVA
{

SoundGroup::SoundGroup() :
animatedVolume(-1.f)
{
	FMOD_VERIFY(SoundSystem::Instance()->fmodSystem->createSoundGroup(0, &fmodSoundGroup));
}

SoundGroup::~SoundGroup()
{
	FMOD_VERIFY(fmodSoundGroup->release());
}

void SoundGroup::SetVolume(float32 volume)
{
	FMOD_VERIFY(fmodSoundGroup->setVolume(volume));
}

float32 SoundGroup::GetVolume()
{
	float32 volume;
	FMOD_VERIFY(fmodSoundGroup->getVolume(&volume));
	return volume;
}

void SoundGroup::Stop()
{
	FMOD_VERIFY(fmodSoundGroup->stop());
}

void SoundGroup::Update()
{
	if(animatedVolume != -1.f)
		SetVolume(animatedVolume);
}

Animation * SoundGroup::VolumeAnimation(float32 newVolume, float32 time, int32 track /*= 0*/)
{
	animatedVolume = GetVolume();
	Animation * a = new LinearAnimation<float32>(this, &animatedVolume, newVolume, time, Interpolation::LINEAR);
	a->AddEvent(Animation::EVENT_ANIMATION_END, Message(this, &SoundGroup::OnVolumeAnimationEnded));
	Retain();
	a->Start(track);

	return a;
}

void SoundGroup::OnVolumeAnimationEnded(BaseObject * caller, void * userData, void * callerData)
{
	SetVolume(animatedVolume);
	animatedVolume = -1.f;
	Release();
}

};