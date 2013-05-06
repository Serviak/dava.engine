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
        * Created by Ivan Petrochenko
=====================================================================================*/

#include "Sound/SoundInstance.h"
#include "Sound/SoundChannel.h"
#include "Sound/SoundSystem.h"
#include "Sound/Sound.h"
#include "Animation/LinearAnimation.h"
#if defined(__DAVAENGINE_IPHONE__)
#include "Sound/MusicIos.h"
#endif


namespace DAVA
{

SoundInstance::SoundInstance()
:	state(STATE_PLAYING),
	animatedVolume(-1.f)
#if defined(__DAVAENGINE_IPHONE__)
    ,buddyMusic(0)
#endif
{
	SoundSystem::Instance()->AddSoundInstance(this);
}

#ifdef __DAVAENGINE_ANDROID__
SoundInstance::SoundInstance(Sound * parent)
:	state(STATE_PLAYING),
    animatedVolume(-1.f),
    parentSound(NULL)
{
	SoundSystem::Instance()->AddSoundInstance(this);
    parentSound = SafeRetain(parent);
}
#endif //#ifdef __DAVAENGINE_ANDROID__
    
SoundInstance::~SoundInstance()
{
#ifdef __DAVAENGINE_ANDROID__
    SafeRelease(parentSound);
#endif //#ifdef __DAVAENGINE_ANDROID__
};

    
SoundInstance::eState SoundInstance::GetState() const
{
	return state;
}

bool SoundInstance::Update()
{
	if(STATE_PLAYING == state)
	{
#if defined(__DAVAENGINE_IPHONE__)
        if(buddyMusic)
        {   
            if(!buddyMusic->IsPlaying())
            {
                soundInstanceEventDispatcher.PerformEvent(EVENT_COMPLETED);

                state = STATE_COMPLETED;
                SoundSystem::Instance()->RemoveSoundInstance(this);
                Release();
                return false;
            }
        }
        else if(SoundChannel::STATE_FREE == buddyChannel->GetState() || state == STATE_FORCED_STOPPED)
		{
			soundInstanceEventDispatcher.PerformEvent(EVENT_COMPLETED);

			state = STATE_COMPLETED;
			SoundSystem::Instance()->RemoveSoundInstance(this);
			buddyChannel->GetBuddySound()->RemoveSoundInstance(this);
			buddyChannel = 0;
            Release();
			return false;
		}
#elif defined(__DAVAENGINE_ANDROID__)
        if(!parentSound)
            return false;

        if(SL_PLAYSTATE_STOPPED == parentSound->GetPlayState() || state == STATE_FORCED_STOPPED)
		{
			soundInstanceEventDispatcher.PerformEvent(EVENT_COMPLETED);
            
			state = STATE_COMPLETED;
			SoundSystem::Instance()->RemoveSoundInstance(this);
            Release();
            
			return false;
		}
#else
		if(SoundChannel::STATE_FREE == buddyChannel->GetState() || state == STATE_FORCED_STOPPED)
		{
			soundInstanceEventDispatcher.PerformEvent(EVENT_COMPLETED);

			state = STATE_COMPLETED;
			SoundSystem::Instance()->RemoveSoundInstance(this);
			buddyChannel->GetBuddySound()->RemoveSoundInstance(this);
			buddyChannel = 0;
			Release();
			return false;
		}
#endif //#if defined(__DAVAENGINE_IPHONE__)
		if(-1.f != animatedVolume)
		{
			SetVolume(animatedVolume);
        }
	}

	return true;
}

void SoundInstance::SetVolume(float32 volume)
{
#if defined(__DAVAENGINE_IPHONE__)
    if(buddyMusic)
    {
        buddyMusic->SetVolume(volume);
    }
    else if(buddyChannel)
    {
		buddyChannel->SetVolume(volume);
    }
#elif defined(__DAVAENGINE_ANDROID__)
    if(parentSound)
    {
        parentSound->SetVolume(volume);
    }
#else
	if(buddyChannel)
		buddyChannel->SetVolume(volume);
#endif //#if defined(__DAVAENGINE_IPHONE__)
}

float32 SoundInstance::GetVolume() const
{
#if defined(__DAVAENGINE_IPHONE__)
    if(buddyMusic)
    {
        return buddyMusic->GetVolume();
    }
    else return buddyChannel ? buddyChannel->GetVolume() : 0.f;
#elif defined(__DAVAENGINE_ANDROID__)
    return parentSound ? parentSound->GetVolume() : 0.f;
#else
	return buddyChannel ? buddyChannel->GetVolume() : 0.f;
#endif //#if defined(__DAVAENGINE_IPHONE__)
}

void SoundInstance::SetPosition(const Vector3 & position)
{
#if defined(__DAVAENGINE_IPHONE__)
	if(buddyChannel)
		buddyChannel->SetPosition(position);
#elif defined(__DAVAENGINE_ANDROID__)
	//
#else
	if(buddyChannel)
		buddyChannel->SetPosition(position);
#endif //#if defined(__DAVAENGINE_IPHONE__)
}


void SoundInstance::SetIgnorePosition(bool ignorePosition)
{
#if defined(__DAVAENGINE_IPHONE__)
	if(buddyChannel)
		buddyChannel->SetIgnorePosition(ignorePosition);
#elif defined(__DAVAENGINE_ANDROID__)
	//
#else
	if(buddyChannel)
		buddyChannel->SetIgnorePosition(ignorePosition);
#endif //#if defined(__DAVAENGINE_IPHONE__)
}

void SoundInstance::Stop()
{
#if defined(__DAVAENGINE_IPHONE__)
    if(buddyMusic)
    {
        buddyMusic->Stop();
    }
    else if(buddyChannel)
	{
		buddyChannel->Stop();
	}
#elif defined(__DAVEENGINE_ANDROID__)
    if(parentSound)
    {
        parentSound->Stop();
    }
#else
	if(buddyChannel)
	{
		buddyChannel->Stop();
	}
#endif //#if defined(__DAVAENGINE_IPHONE__)
	state = STATE_FORCED_STOPPED;
}

Animation * SoundInstance::VolumeAnimation(float32 newVolume, float32 time, int32 track /*= 0*/)
{
	animatedVolume = GetVolume();
	Animation * a = new LinearAnimation<float32>(this, &animatedVolume, newVolume, time, Interpolation::LINEAR);
	a->AddEvent(Animation::EVENT_ANIMATION_END, Message(this, &SoundInstance::OnVolumeAnimationEnded));
	Retain();
	a->Start(track);

	return a;
}

void SoundInstance::OnVolumeAnimationEnded(BaseObject * caller, void * userData, void * callerData)
{
	SetVolume(animatedVolume);
	animatedVolume = -1.f;
	Release();
}

void SoundInstance::Pause(bool pause)
{
#if defined(__DAVAENGINE_IPHONE__)
	if(buddyMusic)
	{
		buddyChannel->Pause(pause);
	}
	else if(buddyChannel)
	{
		buddyChannel->Pause(pause);
	}
#elif defined(__DAVAENGINE_ANDROID__)
    if(parentSound)
    {
        parentSound->Pause(pause);
    }
#else
    if(buddyChannel)
    {
        buddyChannel->Pause(pause);
    }
#endif //#if defined(__DAVAENGINE_IPHONE__)
}




};