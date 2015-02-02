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



#ifndef _ANDROID_LAYER_
#define _ANDROID_LAYER_

#include "AndroidLayer.h"

#include "Platform/TemplateAndroid/CorePlatformAndroid.h"
#include "FileSystem/Logger.h"
#include "Utils/Utils.h"
#include "Input/AccelerometerAndroid.h"
#include "AndroidDelegate.h"
#include "AndroidCrashReport.h"

#include "JniExtensions.h"
#include "WebViewControl.h"
#include "Debug/DVAssertMessageAndroid.h"
#include "Platform/TemplateAndroid/DeviceInfoAndroid.h"
#include "Platform/TemplateAndroid/DateTimeAndroid.h"
#include "Utils/UtilsAndroid.h"
#include "UI/UITextFieldAndroid.h"
#include "Platform/TemplateAndroid/DPIHelperAndroid.h"
#include "Platform/TemplateAndroid/AndroidCrashReport.h"
#include "Platform/TemplateAndroid/MovieViewControlAndroid.h"
#include "FileSystem/LocalizationAndroid.h"
#include "Platform/TemplateAndroid/FileListAndroid.h"
#include "Utils/UTF8Utils.h"

//#if defined(__DAVAENGINE_PROFILE__)
//#include "prof.h"
//#endif //#if defined(__DAVAENGINE_PROFILE__)

extern "C"
{
	jint JNI_OnLoad(JavaVM *vm, void *reserved);

	//JNIApplication
	JNIEXPORT void JNICALL Java_com_dava_framework_JNIApplication_OnCreateApplication(JNIEnv* env, jobject classthis, jstring externalPath, jstring internalPath, jstring apppath, jstring logTag, jstring packageName);
	JNIEXPORT void JNICALL Java_com_dava_framework_JNIApplication_OnConfigurationChanged(JNIEnv * env, jobject classthis);
	JNIEXPORT void JNICALL Java_com_dava_framework_JNIApplication_OnLowMemory(JNIEnv * env, jobject classthis);
	JNIEXPORT void JNICALL Java_com_dava_framework_JNIApplication_OnTerminate(JNIEnv * env, jobject classthis);
 	JNIEXPORT void JNICALL Java_com_dava_framework_JNIApplication_SetAssetManager(JNIEnv * env, jobject classthis, jobject assetManager);
 	
	//FrameworkTestProject
	JNIEXPORT void JNICALL Java_com_dava_framework_JNIActivity_nativeOnCreate(JNIEnv * env, jobject classthis, jboolean isFirstRun);
	JNIEXPORT void JNICALL Java_com_dava_framework_JNIActivity_nativeOnStart(JNIEnv * env, jobject classthis);
	JNIEXPORT void JNICALL Java_com_dava_framework_JNIActivity_nativeOnStop(JNIEnv * env, jobject classthis);
	JNIEXPORT void JNICALL Java_com_dava_framework_JNIActivity_nativeIsFinishing(JNIEnv * env, jobject classthis);
	JNIEXPORT void JNICALL Java_com_dava_framework_JNIActivity_nativeOnDestroy(JNIEnv * env, jobject classthis);
	JNIEXPORT void JNICALL Java_com_dava_framework_JNIActivity_nativeOnAccelerometer(JNIEnv * env, jobject classthis, jfloat x, jfloat y, jfloat z);

	//JNIGLSurfaceView
	JNIEXPORT void JNICALL Java_com_dava_framework_JNIGLSurfaceView_nativeOnInput(JNIEnv * env, jobject classthis, jint action, jint id, jfloat x, jfloat y, jdouble time, jint source, jint tapCount);
	JNIEXPORT void JNICALL Java_com_dava_framework_JNIGLSurfaceView_nativeOnKeyDown(JNIEnv * env, jobject classthis, jint keyCode);
	JNIEXPORT void JNICALL Java_com_dava_framework_JNIGLSurfaceView_nativeOnKeyUp(JNIEnv * env, jobject classthis, jint keyCode);
	JNIEXPORT void JNICALL Java_com_dava_framework_JNIGLSurfaceView_nativeOnGamepadElement(JNIEnv * env, jobject classthis, jint elementKey, jfloat value);
	JNIEXPORT void JNICALL Java_com_dava_framework_JNIGLSurfaceView_nativeOnGamepadConnected(JNIEnv * env, jobject classthis, jint deviceId);
	JNIEXPORT void JNICALL Java_com_dava_framework_JNIGLSurfaceView_nativeOnGamepadDisconnected(JNIEnv * env, jobject classthis, jint deviceId);
	JNIEXPORT void JNICALL Java_com_dava_framework_JNIGLSurfaceView_nativeOnGamepadTriggersDisabled(JNIEnv * env, jobject classthis);

	//JNIRenderer
	JNIEXPORT void JNICALL Java_com_dava_framework_JNIRenderer_nativeResize(JNIEnv * env, jobject classthis, jint w, jint h);
	JNIEXPORT void JNICALL Java_com_dava_framework_JNIRenderer_nativeRender(JNIEnv * env, jobject classthis);
	JNIEXPORT void JNICALL Java_com_dava_framework_JNIRenderer_nativeRenderRecreated(JNIEnv * env, jobject classthis);
	JNIEXPORT void JNICALL Java_com_dava_framework_JNIRenderer_nativeOnResumeView(JNIEnv * env, jobject classthis);
	JNIEXPORT void JNICALL Java_com_dava_framework_JNIRenderer_nativeOnPauseView(JNIEnv * env, jobject classthis, jboolean isLock);
};


DAVA::CorePlatformAndroid *core = NULL;

#define MAX_PATH_SZ 260
char documentsFolderPathEx[MAX_PATH_SZ];
char documentsFolderPathIn[MAX_PATH_SZ];
char folderDocuments[MAX_PATH_SZ];
char assetsFolderPath[MAX_PATH_SZ];
char androidLogTag[MAX_PATH_SZ];
char androidPackageName[MAX_PATH_SZ];

AndroidDelegate *androidDelegate;

jint JNI_OnLoad(JavaVM *vm, void *reserved)
{
	JNIEnv* env;
	if (vm->GetEnv((void **)&env,JNI_VERSION_1_6))
	{
		LOGE("Failed get java environment");
		return -1;
	}

	DAVA::JniExtension::SetJavaClass(env, "com/dava/framework/JNIAssert", &DAVA::JniDVAssertMessage::gJavaClass, &DAVA::JniDVAssertMessage::gJavaClassName);
	DAVA::JniExtension::SetJavaClass(env, "com/dava/framework/JNIUtils", &DAVA::JniUtils::gJavaClass, &DAVA::JniUtils::gJavaClassName);
	DAVA::JniExtension::SetJavaClass(env, "com/dava/framework/JNIDeviceInfo", &DAVA::JniDeviceInfo::gJavaClass, &DAVA::JniDeviceInfo::gJavaClassName);
	DAVA::JniExtension::SetJavaClass(env, "com/dava/framework/JNITextField", &DAVA::JniTextField::gJavaClass, &DAVA::JniTextField::gJavaClassName);
	DAVA::JniExtension::SetJavaClass(env, "com/dava/framework/JNIWebView", &DAVA::JniWebView::gJavaClass, &DAVA::JniWebView::gJavaClassName);
	DAVA::JniExtension::SetJavaClass(env, "com/dava/framework/JNIDpiHelper", &DAVA::JniDpiHelper::gJavaClass, &DAVA::JniDpiHelper::gJavaClassName);
	DAVA::JniExtension::SetJavaClass(env, "com/dava/framework/JNICrashReporter", &DAVA::JniCrashReporter::gJavaClass, &DAVA::JniCrashReporter::gJavaClassName);
	DAVA::JniExtension::SetJavaClass(env, "java/lang/String", &DAVA::JniCrashReporter::gStringClass, NULL);
	DAVA::JniExtension::SetJavaClass(env, "com/dava/framework/JNIMovieViewControl", &DAVA::JniMovieViewControl::gJavaClass, &DAVA::JniMovieViewControl::gJavaClassName);
	DAVA::JniExtension::SetJavaClass(env, "com/dava/framework/JNILocalization", &DAVA::JniLocalization::gJavaClass, &DAVA::JniLocalization::gJavaClassName);
	DAVA::JniExtension::SetJavaClass(env, "com/dava/framework/JNIFileList", &DAVA::JniFileList::gJavaClass, &DAVA::JniFileList::gJavaClassName);
	DAVA::JniExtension::SetJavaClass(env, "com/dava/framework/JNIDateTime", &DAVA::JniDateTime::gJavaClass, &DAVA::JniDateTime::gJavaClassName);
	DAVA::Thread::InitMainThread();


	androidDelegate = new AndroidDelegate(vm);

	DAVA::AndroidCrashReport::Init();


	return JNI_VERSION_1_6;
}

bool CreateStringFromJni(JNIEnv* env, jstring jniString, char *generalString)
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

void CreateStringFromJni(JNIEnv* env, jstring jniString, DAVA::String& string)
{
	const char* utfString = env->GetStringUTFChars(jniString, NULL);
	if (utfString)
	{
		string.assign(utfString);
		env->ReleaseStringUTFChars(jniString, utfString);
	}
}

void CreateWStringFromJni(JNIEnv* env, jstring jniString, DAVA::WideString& string)
{
	const jchar *raw = env->GetStringChars(jniString, 0);
	if (raw)
	{
		jsize len = env->GetStringLength(jniString);
		string.assign(raw, raw + len);
		env->ReleaseStringChars(jniString, raw);
	}
}

jstring CreateJString(JNIEnv* env, const DAVA::WideString& string)
{
	return env->NewStringUTF(DAVA::UTF8Utils::EncodeToUTF8(string).c_str());
}

void InitApplication(JNIEnv * env)
{
	if(!core)
	{
		core = new DAVA::CorePlatformAndroid();
		if(core)
		{
			core->CreateAndroidWindow(documentsFolderPathEx, documentsFolderPathIn, assetsFolderPath, androidLogTag, androidDelegate);
		}
		else
		{
			LOGE("[InitApplication] Can't allocate space for CoreAndroidPlatform");
		}
	}
	else
	{
		DAVA::Logger::Warning("[InitApplication] CoreAndroidPlatform has been created");
	}
}

void DeinitApplication()
{
	if(core)
	{
		core->Quit();
		core->ReleaseSingletons();
		core = NULL;
	}

	SafeDelete(androidDelegate);
}

// CALLED FROM JNIApplication
// private static native void OnCreateApplication();
// private static native void OnConfigurationChanged();
// private static native void OnLowMemory();
// private static native void OnTerminate()

void Java_com_dava_framework_JNIApplication_OnCreateApplication(JNIEnv* env, jobject classthis, jstring externalPath, jstring internalPath, jstring apppath, jstring logTag, jstring packageName)
{
	bool retCreateLogTag = CreateStringFromJni(env, logTag, androidLogTag);
//	LOGI("___ OnCreateApplication __ %d", classthis);

	bool retCreatedExDocuments = CreateStringFromJni(env, externalPath, documentsFolderPathEx);
	bool retCreatedInDocuments = CreateStringFromJni(env, internalPath, documentsFolderPathIn);
	bool retCreatedAssets = CreateStringFromJni(env, apppath, assetsFolderPath);
	bool retCreatePackageName = CreateStringFromJni(env, packageName, androidPackageName);

	InitApplication(env);
}

void Java_com_dava_framework_JNIApplication_OnConfigurationChanged(JNIEnv * env, jobject classthis)
{
	if(core)
	{
//		DAVA::Logger::Info("__ CONFIGURATION CHANGED ___  %p", env);
	}
}

void Java_com_dava_framework_JNIApplication_OnLowMemory(JNIEnv * env, jobject classthis)
{
	if(core)
	{
		DAVA::Logger::Info("__ LOW MEMORY ___  %p", env);
	}

//	DAVA::Logger::Info("-------- DEINIT APPLICATION --------");
//	DeinitApplication();
}
void Java_com_dava_framework_JNIApplication_OnTerminate(JNIEnv * env, jobject classthis)
{
//	LOGI("___ ON TERMINATE ___");
}
// END OF JNIApplication

// CALLED FROM JNIActivity
#include "Utils/HTTPDownloader.h"
void Java_com_dava_framework_JNIActivity_nativeOnCreate(JNIEnv * env, jobject classthis, jboolean isFirstRun)
{
//	LOGI("___ ON CREATE ___ %p, %d;  isFirstRun = %d", env, classthis, isFirstRun);
	if(core)
	{
		core->OnCreateActivity();

//		DAVA::DownloadFileFromURLToDocuments("http://seriouswheels.com/pics-2011/def/2011-Edo-Competition-Mercedes-Benz-SLR-Black-Arrow-Exhaust-1024x768.jpg", "~doc:/device.yaml");
	}
}

void Java_com_dava_framework_JNIActivity_nativeOnStart(JNIEnv * env, jobject classthis)
{
//	LOGI("___ ON START ___ %p, %d", env, classthis);

	if(core)
	{
//#if defined(__DAVAENGINE_PROFILE__)
//
//#define STR_EXPAND(tok) #tok
//#define STR(tok) STR_EXPAND(tok)
//        
//        const char *moduleName = STR(__DAVAENGINE_MODULE_NAME__);
//		LOGI("____MODULE___ ___ %s", moduleName);
//        monstartup(moduleName);
//#endif //#if defined(__DAVAENGINE_PROFILE__)
        
		core->StartVisible();
	}
}

void Java_com_dava_framework_JNIActivity_nativeOnStop(JNIEnv * env, jobject classthis)
{
//	LOGI("___ ON STOP ___ %p, %d", env, classthis);

	if(core)
	{
		core->StopVisible();
        
//#if defined(__DAVAENGINE_PROFILE__)
//        moncleanup();
//#endif //#if defined(__DAVAENGINE_PROFILE__)
	}
}

void Java_com_dava_framework_JNIActivity_nativeIsFinishing(JNIEnv * env, jobject classthis)
{
//	LOGI("___ ON FINISHING ___");
	DeinitApplication();
}


void Java_com_dava_framework_JNIActivity_nativeOnDestroy(JNIEnv * env, jobject classthis)
{
//	LOGI("___ ON DESTROY ___");
	if(core)
	{
		core->OnDestroyActivity();
	}
}

void Java_com_dava_framework_JNIActivity_nativeOnAccelerometer(JNIEnv * env, jobject classthis, jfloat x, jfloat y, jfloat z)
{
//	LOGI("___ ON ACC ___ env = %p, %0.4f, %0.4f, %0.4f", env, x,y,z);
	DAVA::AccelerometerAndroidImpl *accelerometer = (DAVA::AccelerometerAndroidImpl *)DAVA::Accelerometer::Instance();
	if(accelerometer)
	{
		accelerometer->SetAccelerationData(x, y, z);
	}
}


// CALLED FROM JNIGLSurfaceView
void Java_com_dava_framework_JNIGLSurfaceView_nativeOnInput(JNIEnv * env, jobject classthis, jint action, jint id, jfloat x, jfloat y, jdouble time, jint source, jint tapCount)
{
	if(core)
	{
		core->OnInput(action, id, x, y, time, source, tapCount);
	}
}

void Java_com_dava_framework_JNIGLSurfaceView_nativeOnKeyDown(JNIEnv * env, jobject classthis, jint keyCode)
{
	if(core)
	{
		core->KeyDown(keyCode);
	}
}

void Java_com_dava_framework_JNIGLSurfaceView_nativeOnKeyUp(JNIEnv * env, jobject classthis, jint keyCode)
{
	if(core)
	{
		core->KeyUp(keyCode);
	}
}

JNIEXPORT void JNICALL Java_com_dava_framework_JNIGLSurfaceView_nativeOnGamepadElement(JNIEnv * env, jobject classthis, jint elementKey, jfloat value)
{
	if(core)
	{
		core->OnGamepadElement(elementKey, value);
	}
}

JNIEXPORT void JNICALL Java_com_dava_framework_JNIGLSurfaceView_nativeOnGamepadConnected(JNIEnv * env, jobject classthis, jint deviceId)
{
	if(core)
	{
		core->OnGamepadAvailable(true);
	}
}

JNIEXPORT void JNICALL Java_com_dava_framework_JNIGLSurfaceView_nativeOnGamepadDisconnected(JNIEnv * env, jobject classthis, jint deviceId)
{
	if(core)
	{
		core->OnGamepadAvailable(false);
	}
}

JNIEXPORT void JNICALL Java_com_dava_framework_JNIGLSurfaceView_nativeOnGamepadTriggersDisabled(JNIEnv * env, jobject classthis)
{
	if(core)
	{
		core->OnGamepadTriggersDisabled();
	}
}
// END OF JNIGLSurfaceView



// CALLED FROM JNIRenderer

void Java_com_dava_framework_JNIRenderer_nativeOnResumeView(JNIEnv * env, jobject classthis)
{
	if(core)
	{
		core->StartForeground();
	}
}
void Java_com_dava_framework_JNIRenderer_nativeOnPauseView(JNIEnv * env, jobject classthis, jboolean isLock)
{
	if(core)
	{
		core->StopForeground(isLock);
	}
}

void Java_com_dava_framework_JNIRenderer_nativeResize(JNIEnv * env, jobject classthis, jint w, jint h)
{
	if(core)
	{
		LOGI("__ NATIVE RESIZE ___ %d, %d", w, h);
		core->RenderRecreated(w, h);
	}
}

void Java_com_dava_framework_JNIRenderer_nativeRender(JNIEnv * env, jobject classthis)
{
	if(core)
	{
		core->RepaintView();
	}
}

void Java_com_dava_framework_JNIRenderer_nativeRenderRecreated(JNIEnv * env, jobject classthis)
{
	if(core)
	{
		if(androidDelegate)
		{
			androidDelegate->SetBuffers(0, 0);
		}
	}
}

//END OF activity

#endif //#ifndef _ANDROID_LAYER_
