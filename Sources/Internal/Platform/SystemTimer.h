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
        * Created by Vitaliy Borodovsky 
=====================================================================================*/
#ifndef __DAVAENGINE_SYSTEM_TIMER__
#define __DAVAENGINE_SYSTEM_TIMER__

#include "Base/BaseTypes.h"
#include "Base/Singleton.h"

namespace DAVA 
{
#if defined(__DAVAENGINE_ANDROID__)
class Mutex;
#endif //#if defined(__DAVAENGINE_ANDROID__)
class SystemTimer : public Singleton<SystemTimer> 
{
    friend class Core;
#if defined(__DAVAENGINE_WIN32__)
	LARGE_INTEGER	liFrequency;
	LARGE_INTEGER	tLi;
	BOOL			bHighTimerSupport;
	float32			t0;
#elif defined (__DAVAENGINE_ANDROID__)
	float32			t0;
	uint64 savedSec;
#elif defined(__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_MACOS__)
	uint64_t t0;
#else //PLATFORMS
	//other platforms
#endif //PLATFORMS

	static float delta;
	static uint64 stampTime;

	float32 ElapsedSec();
	void Start();
public:
	
	SystemTimer();
	virtual ~SystemTimer();

		uint64 AbsoluteMS();

	static void SetFrameDelta(float32 _delta); //for replay playback only
	
	static float FrameDelta()
	{
		return delta;
	}
	
	static uint64 FrameStampTimeMS()
	{
		return stampTime;
	}
#if defined(__DAVAENGINE_ANDROID__)
	Mutex  *tickMutex;
	uint64 GetTickCount();
	void InitTickCount();
#endif //#if defined(__DAVAENGINE_ANDROID__)
};
};

#endif // #ifndef __DAVAENGINE_SYSTEM_TIMER__
