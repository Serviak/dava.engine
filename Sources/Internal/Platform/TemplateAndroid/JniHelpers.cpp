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

#include "JniHelpers.h"

#if defined(__DAVAENGINE_ANDROID__)
#include "Platform/TemplateAndroid/CorePlatformAndroid.h"
#include "Render/2D/Systems/VirtualCoordinatesSystem.h"

namespace DAVA
{

jstringArray::jstringArray(const jobjectArray &arr)
{
    obj = arr;
}

namespace JNI
{

JavaVM *GetJVM()
{
    CorePlatformAndroid *core = static_cast<DAVA::CorePlatformAndroid *>(Core::Instance());
    AndroidSystemDelegate* delegate = core->GetAndroidSystemDelegate();
    return delegate->GetVM();
}

JNIEnv *GetEnv()
{
    JNIEnv *env;
    JavaVM *vm = GetJVM();

    if (NULL == vm || JNI_EDETACHED == vm->GetEnv((void**)&env, JNI_VERSION_1_6))
    {
        Logger::Error("runtime_error(Thread is not attached to JNI)");
    }
    DVASSERT(NULL != env);
    return env;
}

void AttachCurrentThreadToJVM()
{
	if (true == Thread::IsMainThread())
		return;

	JavaVM *vm = GetJVM();
	JNIEnv *env;

	if (JNI_EDETACHED == vm->GetEnv((void**)&env, JNI_VERSION_1_6))
	{
		if (vm->AttachCurrentThread(&env, NULL)!=0)
			Logger::Error("runtime_error(Could not attach current thread to JNI)");
	}
}

void DetachCurrentThreadFromJVM()
{
	if (true == Thread::IsMainThread())
		return;

	JavaVM *vm = GetJVM();
	JNIEnv *env;
	if (JNI_OK == vm->GetEnv((void**)&env, JNI_VERSION_1_6))
	{
		if (0 != vm->DetachCurrentThread())
			Logger::Error("runtime_error(Could not detach current thread from JNI)");
	}
}

Rect V2P(const Rect& srcRect)
{
    Vector2 offset = VirtualCoordinatesSystem::Instance()->GetPhysicalDrawOffset();
    Rect rect = VirtualCoordinatesSystem::Instance()->ConvertVirtualToPhysical(srcRect);

    rect += offset;
    return rect;
}

bool CreateStringFromJni(JNIEnv *env, jstring jniString, char *generalString)
{
	bool ret = false;
	generalString[0] = 0;

	const char* utfString = env->GetStringUTFChars(jniString, NULL);
	if (utfString)
	{
		strcpy(generalString, utfString);
		env->ReleaseStringUTFChars(jniString, utfString);
		ret = true;
	}
	else
	{
		LOGE("[CreateStringFromJni] Can't create utf-string from jniString");
	}

	return ret;
}

void CreateStringFromJni(JNIEnv *env, jstring jniString, DAVA::String& string)
{
	const char* utfString = env->GetStringUTFChars(jniString, NULL);
	if (utfString)
	{
		string.assign(utfString);
		env->ReleaseStringUTFChars(jniString, utfString);
	}
}

void CreateWStringFromJni(JNIEnv *env, jstring jniString, DAVA::WideString& string)
{
	const jchar *raw = env->GetStringChars(jniString, 0);
	if (raw)
	{
		jsize len = env->GetStringLength(jniString);
		string.assign(raw, raw + len);
		env->ReleaseStringChars(jniString, raw);
	}
}

jstring CreateJString(JNIEnv *env, const DAVA::WideString& string)
{
	return env->NewStringUTF(DAVA::UTF8Utils::EncodeToUTF8(string).c_str());
}

JavaClass::JavaClass(const String &className)
{
    jvm = GetJVM();
    JNIEnv *env = GetEnv();

    name = className;

    jclass classToFind = env->FindClass(name.c_str());
    CheckOperationResult(classToFind, name);

    jclass objectClass = env->GetObjectClass(classToFind);

    jclass classLoaderClass = env->FindClass("java/lang/ClassLoader");
    jmethodID getClassLoaderMethod = env->GetMethodID(objectClass, "getClassLoader", "()Ljava/lang/ClassLoader;");
    jobject gClassLoader = env->CallObjectMethod(classToFind, getClassLoaderMethod);
    jmethodID gFindClassMethod = env->GetMethodID(classLoaderClass, "findClass", "(Ljava/lang/String;)Ljava/lang/Class;");

    jstring jname = env->NewStringUTF(name.c_str());
    jclass foundClass = static_cast<jclass>(env->CallObjectMethod(gClassLoader, gFindClassMethod, jname));
    env->DeleteLocalRef(jname);

    if (NULL != foundClass)
    {
        javaClass = static_cast<jclass>(env->NewGlobalRef(foundClass));
    }

    env->DeleteLocalRef(classToFind);
    env->DeleteLocalRef(objectClass);
    env->DeleteLocalRef(classLoaderClass);
    env->DeleteLocalRef(gClassLoader);
    env->DeleteLocalRef(foundClass);

    CheckOperationResult(javaClass, name);

}

JavaClass::~JavaClass()
{
    GetEnv()->DeleteGlobalRef(javaClass);
}

}


}
#endif
