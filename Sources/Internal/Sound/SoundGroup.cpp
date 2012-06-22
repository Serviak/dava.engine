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

#include "Sound/SoundGroup.h"
#include "Sound/Sound.h"

#ifdef __DAVAENGINE_ANDROID__
#include <SLES/OpenSLES.h>
#endif //#ifdef __DAVAENGINE_ANDROID__

namespace DAVA
{

SoundGroup::SoundGroup()
:	volume(1.f)
{

}

SoundGroup::~SoundGroup()
{
	List<Sound*>::iterator it;
	for(it = sounds.begin(); it != sounds.end(); ++it)
	{
		Sound * sound = *it;
		sound->Release();
	}
}

void SoundGroup::AddSound(Sound * sound)
{
	sound->SetVolume(volume);
	sounds.push_back(sound);
	sound->SetSoundGroup(this);
}

void SoundGroup::RemoveSound(Sound * sound)
{
	sounds.remove(sound);
	sound->SetSoundGroup(0);
}

void SoundGroup::SetVolume(float32 _volume)
{
	volume = Clamp(_volume, 0.f, 1.f);
	List<Sound*>::iterator it;
	for(it = sounds.begin(); it != sounds.end(); ++it)
	{
		Sound * sound = *it;
		sound->SetVolume(volume);
	}
}

float32 SoundGroup::GetVolume()
{
	return volume;
}

#ifdef __DAVAENGINE_ANDROID__
void SoundGroup::Suspend()
{
    List<Sound*>::iterator it;
	for(it = sounds.begin(); it != sounds.end(); ++it)
	{
		Sound * sound = *it;
        if(sound->GetPlayState() == SL_PLAYSTATE_PLAYING)
            sound->Pause(true);
	}
}
void SoundGroup::Resume()
{
    List<Sound*>::iterator it;
	for(it = sounds.begin(); it != sounds.end(); ++it)
	{
		Sound * sound = *it;
        if(sound->GetPlayState() == SL_PLAYSTATE_PAUSED)
            sound->Pause(false);
	}
}
#endif //#ifdef __DAVAENGINE_ANDROID__
    
};