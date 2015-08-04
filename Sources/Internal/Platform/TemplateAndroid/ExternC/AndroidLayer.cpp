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
#include "Platform/TemplateAndroid/AndroidCrashReport.h"
#include "Platform/TemplateAndroid/JniExtensions.h"
#include "Platform/TemplateAndroid/WebViewControlAndroid.h"
#include "Debug/DVAssertMessage.h"
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
#include "Platform/TemplateAndroid/JniHelpers.h"
#include <dirent.h>
//#if defined(__DAVAENGINE_PROFILE__)
//#include "prof.h"
//#endif //#if defined(__DAVAENGINE_PROFILE__)

extern "C"
{
	jint JNI_OnLoad(JavaVM *vm, void *reserved);

	//JNIApplication
	JNIEXPORT void JNICALL Java_com_dava_framework_JNIApplication_OnCreateApplication(JNIEnv* env, jobject classthis, jstring externalPath, jstring internalPath, jstring apppath, jstring logTag, jstring packageName, jstring commandLineParams);
	JNIEXPORT void JNICALL Java_com_dava_framework_JNIApplication_OnConfigurationChanged(JNIEnv * env, jobject classthis);
	JNIEXPORT void JNICALL Java_com_dava_framework_JNIApplication_OnLowMemoryWarning(JNIEnv * env, jobject classthis);
	JNIEXPORT void JNICALL Java_com_dava_framework_JNIApplication_OnTerminate(JNIEnv * env, jobject classthis);
 	JNIEXPORT void JNICALL Java_com_dava_framework_JNIApplication_SetAssetManager(JNIEnv * env, jobject classthis, jobject assetManager);
 	
	//JNIActivity
	JNIEXPORT void JNICALL Java_com_dava_framework_JNIActivity_nativeOnCreate(JNIEnv * env, jobject classthis, jboolean isFirstRun);
	JNIEXPORT void JNICALL Java_com_dava_framework_JNIActivity_nativeOnStart(JNIEnv * env, jobject classthis);
	JNIEXPORT void JNICALL Java_com_dava_framework_JNIActivity_nativeOnStop(JNIEnv * env, jobject classthis);
	JNIEXPORT void JNICALL Java_com_dava_framework_JNIActivity_nativeFinishing(JNIEnv * env, jobject classthis);
	JNIEXPORT void JNICALL Java_com_dava_framework_JNIActivity_nativeOnDestroy(JNIEnv * env, jobject classthis);
	JNIEXPORT void JNICALL Java_com_dava_framework_JNIActivity_nativeOnAccelerometer(JNIEnv * env, jobject classthis, jfloat x, jfloat y, jfloat z);
	JNIEXPORT void JNICALL Java_com_dava_framework_JNIActivity_nativeOnGamepadAvailable(JNIEnv * env, jobject classthis, jboolean isAvailable);
	JNIEXPORT void JNICALL Java_com_dava_framework_JNIActivity_nativeOnGamepadTriggersAvailable(JNIEnv * env, jobject classthis, jboolean isAvailable);
    JNIEXPORT bool JNICALL Java_com_dava_framework_JNIActivity_nativeIsMultitouchEnabled(JNIEnv * env, jobject classthis);

	//JNIGLSurfaceView
	JNIEXPORT void JNICALL Java_com_dava_framework_JNIGLSurfaceView_nativeOnInput(JNIEnv * env, jobject classthis, jint action, jint source, jint groupSize, jobject activeInputs, jobject allInputs);
	JNIEXPORT void JNICALL Java_com_dava_framework_JNIGLSurfaceView_nativeOnKeyDown(JNIEnv * env, jobject classthis, jint keyCode);
	JNIEXPORT void JNICALL Java_com_dava_framework_JNIGLSurfaceView_nativeOnKeyUp(JNIEnv * env, jobject classthis, jint keyCode);
	JNIEXPORT void JNICALL Java_com_dava_framework_JNIGLSurfaceView_nativeOnGamepadElement(JNIEnv * env, jobject classthis, jint elementKey, jfloat value, jboolean isKeycode);

	//JNIRenderer
	JNIEXPORT void JNICALL Java_com_dava_framework_JNIRenderer_nativeResize(JNIEnv * env, jobject classthis, jint w, jint h);
	JNIEXPORT void JNICALL Java_com_dava_framework_JNIRenderer_nativeRender(JNIEnv * env, jobject classthis);
	JNIEXPORT void JNICALL Java_com_dava_framework_JNIRenderer_nativeRenderRecreated(JNIEnv * env, jobject classthis);
	JNIEXPORT void JNICALL Java_com_dava_framework_JNIRenderer_nativeOnResumeView(JNIEnv * env, jobject classthis);
	JNIEXPORT void JNICALL Java_com_dava_framework_JNIRenderer_nativeOnPauseView(JNIEnv * env, jobject classthis, jboolean isLock);
};

#define MAX_PATH_SZ 260

namespace 
{
	DAVA::CorePlatformAndroid *core = NULL;

	char documentsFolderPathEx[MAX_PATH_SZ];
	char documentsFolderPathIn[MAX_PATH_SZ];
	char folderDocuments[MAX_PATH_SZ];
	char assetsFolderPath[MAX_PATH_SZ];
	char androidLogTag[MAX_PATH_SZ];
	char androidPackageName[MAX_PATH_SZ];

	DAVA::JNI::JavaClass* gArrayListClass = nullptr;
	DAVA::JNI::JavaClass* gInputEventClass = nullptr;

	DAVA::Function< jobject(jobject, jint) > gArrayListGetMethod;
	DAVA::Function< jint(jobject) > gArrayListSizeMethod;

	jfieldID gInputEventTidField;
	jfieldID gInputEventXField;
	jfieldID gInputEventYField;
	jfieldID gInputEventTimeField;
	jfieldID gInputEventTapCountField;

	AndroidDelegate *androidDelegate;
}

jint JNI_OnLoad(JavaVM *vm, void *reserved)
{
	JNIEnv* env;
	if (vm->GetEnv((void **)&env,JNI_VERSION_1_6))
	{
		LOGE("Failed get java environment");
		return -1;
	}

	DAVA::Thread::InitMainThread();

	androidDelegate = new AndroidDelegate(vm);

	 DAVA::AndroidCrashReport::Init(env);

	return JNI_VERSION_1_6;
}

void InitApplication(JNIEnv * env, const DAVA::String& commandLineParams)
{
	if(!core)
	{
		core = new DAVA::CorePlatformAndroid(commandLineParams);
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

void Java_com_dava_framework_JNIApplication_OnCreateApplication(JNIEnv* env, jobject classthis, jstring externalPath, jstring internalPath, jstring apppath, jstring logTag, jstring packageName, jstring commandLineParams)
{
	bool retCreateLogTag = DAVA::JNI::CreateStringFromJni(env, logTag, androidLogTag);
//	LOGI("___ OnCreateApplication __ %d", classthis);

	bool retCreatedExDocuments = DAVA::JNI::CreateStringFromJni(env, externalPath, documentsFolderPathEx);
	bool retCreatedInDocuments = DAVA::JNI::CreateStringFromJni(env, internalPath, documentsFolderPathIn);
	bool retCreatedAssets = DAVA::JNI::CreateStringFromJni(env, apppath, assetsFolderPath);
	bool retCreatePackageName = DAVA::JNI::CreateStringFromJni(env, packageName, androidPackageName);
	DAVA::String commandLine;
	DAVA::JNI::CreateStringFromJni(env, commandLineParams, commandLine);

	InitApplication(env, commandLine);

	gArrayListClass = new DAVA::JNI::JavaClass("java/util/ArrayList");
	gInputEventClass = new DAVA::JNI::JavaClass("com/dava/framework/JNIGLSurfaceView$InputRunnable$InputEvent");

	gArrayListGetMethod = gArrayListClass->GetMethod<jobject, jint>("get");
	gArrayListSizeMethod = gArrayListClass->GetMethod<jint>("size");

	gInputEventTidField = env->GetFieldID(*gInputEventClass, "tid", DAVA::JNI::TypeMetrics<jint>());
	gInputEventXField = env->GetFieldID(*gInputEventClass, "x", DAVA::JNI::TypeMetrics<jfloat>());
	gInputEventYField = env->GetFieldID(*gInputEventClass, "y", DAVA::JNI::TypeMetrics<jfloat>());
	gInputEventTimeField = env->GetFieldID(*gInputEventClass, "time", DAVA::JNI::TypeMetrics<jdouble>());
	gInputEventTapCountField = env->GetFieldID(*gInputEventClass, "tapCount", DAVA::JNI::TypeMetrics<jint>());


}

void Java_com_dava_framework_JNIApplication_OnConfigurationChanged(JNIEnv * env, jobject classthis)
{
	if(core)
	{
//		DAVA::Logger::Info("__ CONFIGURATION CHANGED ___  %p", env);
	}
}

void Java_com_dava_framework_JNIApplication_OnLowMemoryWarning(JNIEnv * env, jobject classthis)
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

void Java_com_dava_framework_JNIActivity_nativeFinishing(JNIEnv * env, jobject classthis)
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

	DAVA::SafeDelete(gArrayListClass);
	DAVA::SafeDelete(gInputEventClass);
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

void Java_com_dava_framework_JNIActivity_nativeOnGamepadAvailable(JNIEnv * env, jobject classthis, jboolean isAvailable)
{
	if(core)
	{
		core->OnGamepadAvailable(isAvailable);
	}
}

void Java_com_dava_framework_JNIActivity_nativeOnGamepadTriggersAvailable(JNIEnv * env, jobject classthis, jboolean isAvailable)
{
	if(core)
	{
		core->OnGamepadTriggersAvailable(isAvailable);
	}
}

namespace
{
	DAVA::int32 GetPhase(DAVA::int32 action, DAVA::int32 source)
	{
		DAVA::int32 phase = DAVA::UIEvent::PHASE_DRAG;
		switch(action)
		{
			case 5: //ACTION_POINTER_DOWN
			case 0://ACTION_DOWN
			phase = DAVA::UIEvent::PHASE_BEGAN;
			break;

			case 6://ACTION_POINTER_UP
			case 1://ACTION_UP
			phase = DAVA::UIEvent::PHASE_ENDED;
			break;

			case 2://ACTION_MOVE
			{
				if((source & 0x10) > 0)//SOURCE_CLASS_JOYSTICK
				{
					phase = DAVA::UIEvent::PHASE_JOYSTICK;
				}
				else //Touches
					phase = DAVA::UIEvent::PHASE_DRAG;
			}
			break;

			case 3://ACTION_CANCEL
			phase = DAVA::UIEvent::PHASE_CANCELLED;
			break;

			case 4://ACTION_OUTSIDE
			break;
		}

		return phase;
	}

	DAVA::UIEvent CreateUIEventFromJavaEvent(JNIEnv * env, jobject input, jint action, jint source)
	{
		DAVA::UIEvent event;
		event.tid = env->GetIntField(input, gInputEventTidField);
		event.point.x = event.physPoint.x = env->GetFloatField(input, gInputEventXField);
		event.point.y = event.physPoint.y = env->GetFloatField(input, gInputEventYField);
		event.tapCount = env->GetIntField(input, gInputEventTapCountField);
		event.timestamp = env->GetDoubleField(input, gInputEventTimeField);

		return event;
	}
}

// CALLED FROM JNIGLSurfaceView

void Java_com_dava_framework_JNIGLSurfaceView_nativeOnInput(JNIEnv * env, jobject classthis, jint action, jint source, jint groupSize, jobject javaActiveInputs, jobject javaAllInputs)
{
	//action, activeEvents, allEvents, time

	if(core)
	{
		DAVA::Vector< DAVA::UIEvent > activeInputs;
		DAVA::Vector< DAVA::UIEvent > allInputs;

		int allInputsCount = gArrayListSizeMethod(javaAllInputs);
		int activeInputsCount = gArrayListSizeMethod(javaActiveInputs);

		int inputsCount = DAVA::Max(allInputsCount, activeInputsCount);

		for(int groupStartIndex = 0; groupStartIndex < inputsCount; groupStartIndex += groupSize)
		{
			int groupEndIndex = groupStartIndex + groupSize;

			allInputs.clear();
			activeInputs.clear();

			for (int touchIndex = groupStartIndex; touchIndex < groupEndIndex; ++touchIndex)
			{
				if (touchIndex < allInputsCount)
				{
					jobject jInput = gArrayListGetMethod(javaAllInputs, touchIndex);

					DAVA::UIEvent event = CreateUIEventFromJavaEvent(env, jInput, action, source);
					event.phase = DAVA::UIEvent::PHASE_DRAG;
					allInputs.push_back(event);
				}
				if (touchIndex < activeInputsCount)
				{
					jobject jInput = gArrayListGetMethod(javaActiveInputs, touchIndex);

					DAVA::UIEvent event = CreateUIEventFromJavaEvent(env, jInput, action, source);
					event.phase = GetPhase(action, source);
					activeInputs.push_back(event);
				}
			}
			core->OnInput(action, source, activeInputs, allInputs);
		}
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

void Java_com_dava_framework_JNIGLSurfaceView_nativeOnGamepadElement(JNIEnv * env, jobject classthis, jint elementKey, jfloat value, jboolean isKeycode)
{
	if(core)
	{
		core->OnGamepadElement(elementKey, value, isKeycode);
	}
}

JNIEXPORT bool JNICALL Java_com_dava_framework_JNIActivity_nativeIsMultitouchEnabled(JNIEnv * env, jobject classthis)
{
    if(core)
    {
        return core->IsMultitouchEnabled();
    }
    return true;
}
// END OF JNIGLSurfaceView



// CALLED FROM JNIRenderer

void Java_com_dava_framework_JNIRenderer_nativeOnResumeView(JNIEnv * env, jobject classthis)
{
    DAVA::Thread::InitGLThread();
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
