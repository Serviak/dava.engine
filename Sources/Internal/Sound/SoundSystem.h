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

#ifndef __DAVAENGINE_SOUND_SYSTEM_H__
#define __DAVAENGINE_SOUND_SYSTEM_H__

#include "Base/Singleton.h"
#include "Base/BaseTypes.h"
#include "Base/BaseObject.h"
#include "Base/FastName.h"
#include "FileSystem/FilePath.h"
#include "Base/EventDispatcher.h"
#include "Base/FastName.h"
#include "Sound/SoundEvent.h"

#ifdef DAVA_FMOD
namespace FMOD
{
class EventGroup;
class System;
class EventSystem;
class EventProject;
};
#endif

namespace DAVA
{

#ifdef DAVA_FMOD
class FMODFileSoundEvent;
class FMODSoundEvent;
#endif

class Component;
class SoundSystem : public Singleton<SoundSystem>
{
public:
    SoundSystem();
    ~SoundSystem();
    
    SoundEvent * CreateSoundEventByID(const FastName & eventName, const FastName & groupName);
    SoundEvent * CreateSoundEventFromFile(const FilePath & fileName, const FastName & groupName, uint32 createFlags = SoundEvent::SOUND_EVENT_CREATE_DEFAULT, int32 priority = 128);
    
    void SerializeEvent(const SoundEvent * sEvent, KeyedArchive *toArchive);
    SoundEvent * DeserializeEvent(KeyedArchive *archive);


    void Update(float32 timeElapsed);
    void Suspend();
    void Resume();

    void SetCurrentLocale(const String & langID);

    void SetListenerPosition(const Vector3 & position);
    void SetListenerOrientation(const Vector3 & forward, const Vector3 & left);

    void SetGroupVolume(const FastName & groupName, float32 volume);
    float32 GetGroupVolume(const FastName & groupName);

#ifdef DAVA_FMOD
protected:
    struct SoundGroup
    {
        SoundGroup() : volume(1.f) {};

        FastName name;
        float32 volume;
        Vector<SoundEvent *> events;
    };

public:
    void LoadFEV(const FilePath & filePath);
    void UnloadFEV(const FilePath & filePath);
    void UnloadFMODProjects();

    void PreloadFMODEventGroupData(const String & groupName);
    void ReleaseFMODEventGroupData(const String & groupName);
    void ReleaseAllEventWaveData();

    void GetAllEventsNames(Vector<String> & names);

    uint32 GetMemoryUsageBytes() const;

protected:
    void GetGroupEventsNamesRecursive(FMOD::EventGroup * group, String & currNamePath, Vector<String> & names);

    void ReleaseOnUpdate(SoundEvent * sound);

    void PerformCallbackOnUpdate(SoundEvent * event, uint32 callbackType);
    void CancelCallbackOnUpdate(SoundEvent * event, uint32 callbackType);

    void AddSoundEventToGroup(const FastName & groupName, SoundEvent * event);
    void RemoveSoundEventFromGroups(SoundEvent * event);

    FMOD::System * fmodSystem;
    FMOD::EventSystem * fmodEventSystem;

    Vector<SoundEvent *> soundsToReleaseOnUpdate;
    MultiMap<SoundEvent *, uint32> callbackOnUpdate;
    Vector<SoundGroup> soundGroups;

    Map<FilePath, FMOD::EventProject *> projectsMap;

    Vector<String> toplevelGroups;

    friend class FMODFileSoundEvent;
    friend class FMODSoundEvent;
#endif
};

};

#endif //__DAVAENGINE_SOUND_SYSTEM_H__
