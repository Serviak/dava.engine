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

#include "Sound/FMODSimpleSoundEvent.h"
#include "Sound/FMODSoundSystem.h"
#include "Sound/FMODSoundGroup.h"
#include "Sound/FMODUtils.h"
#include "Scene3D/Entity.h"

namespace DAVA
{
Map<String, FMOD::Sound*> soundMap;
Map<FMOD::Sound*, int32> soundRefsMap;

FMOD_RESULT F_CALLBACK SoundInstanceEndPlaying(FMOD_CHANNEL *channel, FMOD_CHANNEL_CALLBACKTYPE type, void *commanddata1, void *commanddata2);

SimpleSoundEvent * FMODSimpleSoundEvent::CreateWithFlags(const FilePath & fileName, eType type, const FastName & groupName, int32 flags, int32 priority)
{
	FMODSimpleSoundEvent * sound = new FMODSimpleSoundEvent(fileName, type, priority);

	if(flags & FMOD_3D)
		sound->is3d = true;
    Map<String, FMOD::Sound*>::iterator it;
    it = soundMap.find(fileName.GetAbsolutePathname());
    if (it != soundMap.end())
    {
        sound->fmodSound = it->second;
        soundRefsMap[sound->fmodSound]++;
    }

    FMODSoundSystem * soundSystem = FMODSoundSystem::GetFMODSoundSystem();

    if(!sound->fmodSound)
    {
        File * file = File::Create(fileName, File::OPEN | File::READ);
        if(!file)
        {
            SafeRelease(sound);
            return 0;
        }

        int32 fileSize = file->GetSize();
        if(!fileSize)
        {
            SafeRelease(sound);
            return 0;
        }

        sound->soundData = new uint8[fileSize];
        file->Read(sound->soundData, fileSize);
        SafeRelease(file);

        FMOD_CREATESOUNDEXINFO exInfo;
        memset(&exInfo, 0, sizeof(FMOD_CREATESOUNDEXINFO));
        exInfo.cbsize = sizeof(FMOD_CREATESOUNDEXINFO);
        exInfo.length = fileSize;

	    switch (type)
	    {
	    case TYPE_STATIC:
            FMOD_VERIFY(soundSystem->fmodSystem->createSound((char *)sound->soundData, FMOD_LOOP_NORMAL | FMOD_OPENMEMORY | flags, &exInfo, &sound->fmodSound));
            SafeDelete(sound->soundData);
		    break;
	    case TYPE_STREAMED:
		    FMOD_VERIFY(soundSystem->fmodSystem->createStream((char *)sound->soundData, FMOD_LOOP_NORMAL | FMOD_OPENMEMORY | flags, &exInfo, &sound->fmodSound));
		    break;
	    }

        if(!sound->fmodSound)
        {
            SafeRelease(sound);
            return 0;
        }

    #if !defined DONT_USE_DEFAULT_3D_SOUND_SETTINGS
        if( sound->is3d && sound->fmodSound )
            FMOD_VERIFY( sound->fmodSound->set3DMinMaxDistance(5.0f, 100.0f) );
    #endif

	    sound->SetSoundGroup(groupName);
	    sound->SetLoopCount(0);

        soundMap[sound->fileName.GetAbsolutePathname()] = sound->fmodSound;
        soundRefsMap[sound->fmodSound] = 1;
    }
	FMOD_VERIFY(soundSystem->fmodSystem->createChannelGroup(0, &sound->fmodInstanceGroup));

	return sound;
}
FMODSimpleSoundEvent::FMODSimpleSoundEvent(const FilePath & _fileName, eType _type, int32 _priority) : SimpleSoundEvent(_type),
    fileName(_fileName),
	priority(_priority),
	is3d(false),
    soundData(0),
    fmodSound(0),
    fmodInstanceGroup(0)
{
}

FMODSimpleSoundEvent::~FMODSimpleSoundEvent()
{
    SafeDeleteArray(soundData);

    FMOD_VERIFY(fmodInstanceGroup->release());
}

int32 FMODSimpleSoundEvent::Release()
{
    if(GetRetainCount() == 1)
    {
        soundRefsMap[fmodSound]--;
        if(soundRefsMap[fmodSound] == 0)
        {
            soundMap.erase(fileName.GetAbsolutePathname());
            soundRefsMap.erase(fmodSound);
            FMOD_VERIFY(fmodSound->release());
        }
    }

    return BaseObject::Release();
}

void FMODSimpleSoundEvent::SetSoundGroup(const FastName & groupName)
{
	FMODSoundGroup * soundGroup = FMODSoundSystem::GetFMODSoundSystem()->CreateSoundGroup(groupName);
	if(soundGroup)
	{
		FMOD_VERIFY(fmodSound->setSoundGroup(soundGroup->fmodSoundGroup));
	}
}

bool FMODSimpleSoundEvent::Trigger()
{
    FMOD::Channel * fmodInstance = 0;
    FMOD_VERIFY(FMODSoundSystem::GetFMODSoundSystem()->fmodSystem->playSound(FMOD_CHANNEL_FREE, fmodSound, true, &fmodInstance)); //start sound paused
    FMOD_VECTOR pos = {position.x, position.y, position.z};
    FMOD_VERIFY(fmodInstance->setPriority(priority));
    FMOD_VERIFY(fmodInstance->setCallback(SoundInstanceEndPlaying));
    FMOD_VERIFY(fmodInstance->setUserData(this));
    FMOD_VERIFY(fmodInstance->setChannelGroup(fmodInstanceGroup));

    if(is3d)
        FMOD_VERIFY(fmodInstance->set3DAttributes(&pos, 0));

    FMOD_VERIFY(fmodInstance->setPaused(false));

    Retain();

    return true;
}

void FMODSimpleSoundEvent::SetPosition(const Vector3 & _position)
{
	position = _position;
}

void FMODSimpleSoundEvent::UpdateInstancesPosition()
{
	if(is3d)
	{
		FMOD_VECTOR pos = {position.x, position.y, position.z};
		int32 instancesCount = 0;
		FMOD_VERIFY(fmodInstanceGroup->getNumChannels(&instancesCount));
		for(int32 i = 0; i < instancesCount; i++)
		{
			FMOD::Channel * inst = 0;
			FMOD_VERIFY(fmodInstanceGroup->getChannel(i, &inst));
			FMOD_VERIFY(inst->set3DAttributes(&pos, 0));
		}
	}
}

void FMODSimpleSoundEvent::SetVolume(float32 volume)
{
	FMOD_VERIFY(fmodInstanceGroup->setVolume(volume));
}

float32	FMODSimpleSoundEvent::GetVolume()
{
	float32 volume = 0;
	FMOD_VERIFY(fmodInstanceGroup->getVolume(&volume));
	return volume;
}

bool FMODSimpleSoundEvent::IsActive()
{
    //TODO
	bool isPaused = false;
	FMOD_VERIFY(fmodInstanceGroup->getPaused(&isPaused));
	return isPaused;
}

void FMODSimpleSoundEvent::Stop()
{
    FMOD_VERIFY(fmodInstanceGroup->stop());
}

int32 FMODSimpleSoundEvent::GetLoopCount()
{
	int32 loopCount;
	FMOD_VERIFY(fmodSound->getLoopCount(&loopCount));
	return loopCount;
}

void FMODSimpleSoundEvent::SetLoopCount(int32 loopCount)
{
	FMOD_VERIFY(fmodSound->setLoopCount(loopCount));
}

void FMODSimpleSoundEvent::PerformCallback(FMOD::Channel * instance)
{
    PerformEvent(EVENT_END);

    FMODSoundSystem::GetFMODSoundSystem()->ReleaseOnUpdate(this);
}

FMOD_RESULT F_CALLBACK SoundInstanceEndPlaying(FMOD_CHANNEL *channel, FMOD_CHANNEL_CALLBACKTYPE type, void *commanddata1, void *commanddata2)
{
	if(type == FMOD_CHANNEL_CALLBACKTYPE_END)
	{
		FMOD::Channel *cppchannel = (FMOD::Channel *)channel;
        if(cppchannel)
        {
            FMODSimpleSoundEvent * sound = 0;
            FMOD_VERIFY(cppchannel->getUserData((void**)&sound));
            if(sound)
                sound->PerformCallback(cppchannel);
        }
	}

	return FMOD_OK;
}

};
