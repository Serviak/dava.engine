/*==================================================================================
    Copyright (c) 2008, DAVA, INC
    All rights reserved.

    Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the DAVA, INC nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE DAVA, INC AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL DAVA, INC BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/
#ifndef __DAVAENGINE_THREAD_H__
#define __DAVAENGINE_THREAD_H__ 

#include "Base/BaseTypes.h"
#include "Base/Message.h"
#include "Base/BaseObject.h"

#if defined (__DAVAENGINE_ANDROID__)
	#include <EGL/eglplatform.h>
	#include <EGL/egl.h>
#endif //#if defined (__DAVAENGINE_ANDROID__)

namespace DAVA
{
/**
	\defgroup threads Thread wrappers
*/

/**
	\ingroup threads
	\brief wrapper class to give us level of abstraction on thread implementation in particual OS. Now is supports Win32, MacOS, iPhone platforms.
*/
    
class Thread : public BaseObject
{
public:
	enum eThreadState
	{
		STATE_CREATED = 0,
		STATE_RUNNING,
		STATE_ENDED
	};
	
	/**
		\brief static function to detect if current thread is main thread of application
		\returns true if now main thread executes
	*/
	static	bool IsMainThread();

	/**
		\brief static function to create instance of thread object based on Message.

		This function create thread based on message. It do not start the thread until Start function called.

		\returns ptr to thread object 
	*/
	static Thread		* Create(const Message& msg);

	/**
		\brief start execution of the thread
	*/
	void			Start();
	/**
		\brief get current thread state. 

		This function return state of the thread. It can be STATE_CREATED, STATE_RUNNING, STATE_ENDED.
	*/
	eThreadState	GetState();

	/**
		\brief this function is needed to copy gl context from calling thread to this thread
		If you call it before thread start it will copy OpenGL context to this thread
		It will do the same thing with DirectX device, so main purpose of the function is to make GL / DX objects
	*/
	void			EnableCopyContext() { needCopyContext = true; }//setting current context in new thread

    /**
     \brief Notifies the scheduler that the current thread is
     willing to release its processor to other threads of the same or higher
     priority.
     */
    static void YieldThread();
    
private:
	Thread() {};
	Thread(const Thread& t);
	Thread(const Message& msg);
	
	Message			msg;
	eThreadState		state;
	bool				needCopyContext;
	
#if defined(__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_MACOS__)
	void		* glContext;
	friend void	* PthreadMain(void * param);
	void		StartMacOS();
	static void	InitMacOS();
#elif defined (__DAVAENGINE_WIN32__)

	static DWORD	mainThreadId;

public:
	static HDC		currentDC;
	static HGLRC	secondaryContext;

private:
	void		StartWin32();
	HANDLE		handle;
	DWORD		tid;
	friend DWORD WINAPI ThreadFunc(void* param);
public:
	/* 
		Note: This function called from Core::Create. Core::Create must be always called from main thread.
	*/
	static void		InitMainThread();


#elif defined(__DAVAENGINE_ANDROID__)
private:
	friend void	* PthreadMain(void * param);
	void		StartAndroid();
    
    static pid_t mainThreadId;
    
	static EGLContext currentContext;
	static EGLDisplay currentDisplay;
	static EGLSurface currentDrawSurface;
	static EGLSurface currentReadSurface;

	EGLContext localContext;

public:

	static void		InitMainThread();
	static void		SleepThread(int32 timems);

	#else //PLATFORMS
	// other platforms
#endif //PLATFORMS	
};

};

#endif // __DAVAENGINE_THREAD_H__
