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

#ifndef __ANDROID_DELEGATE_H__
#define __ANDROID_DELEGATE_H__

#include "Base/BaseTypes.h"
#if defined(__DAVAENGINE_ANDROID__)

#include <jni.h>
#include "Platform/TemplateAndroid/CorePlatformAndroid.h"

class AndroidDelegate: public DAVA::AndroidSystemDelegate
{
	jobject classApplication;
	jobject classActivity;

	enum eConst
	{
		MAX_PATH_SZ = 260
	};

	char activityName[MAX_PATH_SZ];
	char httpDownloaderName[MAX_PATH_SZ];
    
    GLint frameBuffer;
    GLint renderBuffer;

public:

	AndroidDelegate(JavaVM *vm);

	void SetApplication(jobject app, char *packageName);
	void SetActivity(jobject activity);
    void SetBuffers(GLint newFrameBuffer, GLint newRenderBuffer);

    virtual GLint RenderBuffer();
	virtual GLint FrameBuffer();
	virtual void ShowKeyboard();
	virtual void HideKeyboard();
	virtual bool DownloadHttpFile(const DAVA::String & url, const DAVA::String & documentsPathname);
};

#endif //#if defined(__DAVAENGINE_ANDROID__)

#endif //#ifndef __ANDROID_LISTENER_H__
