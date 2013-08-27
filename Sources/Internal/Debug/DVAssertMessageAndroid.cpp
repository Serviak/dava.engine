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

#include "DVAssertMessage.h"
#include "DVAssertMessageAndroid.h"
#include "Platform/TemplateAndroid/CorePlatformAndroid.h"
#include "ExternC/AndroidLayer.h"

using namespace DAVA;

jclass JniDVAssertMessage::gJavaClass = NULL;
const char* JniDVAssertMessage::gJavaClassName = NULL;

jclass JniDVAssertMessage::GetJavaClass() const
{
	return gJavaClass;
}

const char* JniDVAssertMessage::GetJavaClassName() const
{
	return gJavaClassName;
}


void JniDVAssertMessage::ShowMessage(const char* message)
{
	jmethodID mid = GetMethodID("Assert", "(Ljava/lang/String;)V");
	if (mid)
	{
		jstring jStrMessage = GetEnvironment()->NewStringUTF(message);
		GetEnvironment()->CallStaticVoidMethod(GetJavaClass(), mid, jStrMessage);
		GetEnvironment()->DeleteLocalRef(jStrMessage);
	}
}


void DVAssertMessage::InnerShow(eModalType modalType, const char* message)
{
	JniDVAssertMessage msg;
	msg.ShowMessage(message);
}
