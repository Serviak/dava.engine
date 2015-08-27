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


#include "Platform/TemplateAndroid/CorePlatformAndroid.h"

//#include "Core/Core.h"

#if defined(__DAVAENGINE_ANDROID__)

extern void FrameworkDidLaunched();
extern void FrameworkWillTerminate();

#include "Platform/DeviceInfo.h"
#include "Input/InputSystem.h"
#include "UI/UIEvent.h"
#include "FileSystem/FileSystem.h"
#include "Scene3D/SceneCache.h"
#include "Platform/TemplateAndroid/AssetsManagerAndroid.h"
#include "Render/2D/Systems/RenderSystem2D.h"
#include "Platform/TemplateAndroid/JniHelpers.h"

namespace DAVA
{
	AndroidSystemDelegate::AndroidSystemDelegate(JavaVM *vm)
	{
		Logger::Debug("AndroidSystemDelegate::AndroidSystemDelegate()");

		this->vm = vm;
		environment = NULL;
		if (vm->GetEnv((void**)&environment, JNI_VERSION_1_4) != JNI_OK)
		{
			Logger::Debug("Failed to get the environment using GetEnv()");
		}
	}

	Core::eDeviceFamily Core::GetDeviceFamily()
	{
		float32 width = VirtualCoordinatesSystem::Instance()->GetPhysicalScreenSize().dx;
		float32 height = VirtualCoordinatesSystem::Instance()->GetPhysicalScreenSize().dy;
		float32 dpi = GetScreenDPI();

		float32 inches = sqrt((width * width) + (height * height)) / dpi;

		if (inches > 6.f)
			return DEVICE_PAD;

		return DEVICE_HANDSET;
	}

	CorePlatformAndroid::CorePlatformAndroid(const String& cmdLine)
	: Core()
	{
		wasCreated = false;
		renderIsActive = false;
		width = 0;
		height = 0;
		screenOrientation = Core::SCREEN_ORIENTATION_PORTRAIT; //no need rotate GL for Android

		foreground = false;

		SetCommandLine(cmdLine);
	}

	int Core::Run(int argc, char * argv[], AppHandle handle)
	{
// 		CoreWin32Platform * core = new CoreWin32Platform();
// 		core->CreateWin32Window(handle);
		// 		core->Run();
// 		core->ReleaseSingletons();
		return 0;
	}

	void CorePlatformAndroid::Quit()
	{
	    Logger::Debug("[CorePlatformAndroid::Quit]");
	    QuitAction();

	    renderIsActive = false;
	    // finish java activity
	    JNI::JavaClass javaClass("com/dava/framework/JNIActivity");
	    Function<void()> finishActivity = javaClass.GetStaticMethod<void>("finishActivity");
	    finishActivity();
	}

	void CorePlatformAndroid::QuitAction()
	{
		Logger::Debug("[CorePlatformAndroid::QuitAction]");

		if(Core::Instance())
		{
		    // will call gameCore->onAppFinished() destroy game singletons
			Core::Instance()->SystemAppFinished();
		}

		FrameworkWillTerminate();

		Logger::Debug("[CorePlatformAndroid::QuitAction] done");
	}

	void CorePlatformAndroid::RepaintView()
	{
	    if(renderIsActive)
	    {
	        auto sysTimer = SystemTimer::Instance();
	        auto sysRender = RenderManager::Instance();
	        //  Control FPS
	        {
	            // we count full frame time once per cycle
	            // C++->Java->C++(frame ended)
	            static uint64 startTime = sysTimer->AbsoluteMS();

	            uint64 elapsedTime = sysTimer->AbsoluteMS() - startTime;
	            int32 fpsLimit = sysRender->GetFPS();
	            if (fpsLimit > 0)
	            {
	                uint64 averageFrameTime = 1000UL / static_cast<uint64>(fpsLimit);
	                if(averageFrameTime > elapsedTime)
	                {
	                    uint64 sleepMs = averageFrameTime - elapsedTime;
	                    Thread::Sleep(static_cast<uint32>(sleepMs));
	                }
	            }
	            startTime = sysTimer->AbsoluteMS();
	        }

	        sysRender->Lock();
	        Core::SystemProcessFrame();
	        sysRender->Unlock();
	    }
	}

	void CorePlatformAndroid::ResizeView(int32 w, int32 h)
	{
		width = w;
		height = h;
		DeviceInfo::InitializeScreenInfo();

		UpdateScreenMode();
	}

	void CorePlatformAndroid::UpdateScreenMode()
	{
		Logger::Debug("[CorePlatformAndroid::UpdateScreenMode] start");
		VirtualCoordinatesSystem::Instance()->SetInputScreenAreaSize(width, height);
		VirtualCoordinatesSystem::Instance()->SetPhysicalScreenSize(width, height);

		RenderManager::Instance()->InitFBSize(width, height);
        RenderManager::Instance()->Init(width, height);

		Logger::Debug("[CorePlatformAndroid::] w = %d, h = %d", width, height);
		Logger::Debug("[CorePlatformAndroid::UpdateScreenMode] done");
	}

	void CorePlatformAndroid::CreateAndroidWindow(const char8 *docPathEx, const char8 *docPathIn, const char8 *assets, const char8 *logTag, AndroidSystemDelegate * sysDelegate)
	{
		androidDelegate = sysDelegate;
		externalStorage = docPathEx;
		internalStorage = docPathIn;
	
		Core::CreateSingletons();

		AssetsManager::Instance()->Init(assets);

		Logger::SetTag(logTag);
	}

	void CorePlatformAndroid::RenderRecreated(int32 w, int32 h)
	{
		Logger::Debug("[CorePlatformAndroid::RenderRecreated] start");

		renderIsActive = true;

		Thread::InitGLThread();

		if(wasCreated)
		{
			RenderManager::Instance()->Lost();
			RenderResource::SaveAllResourcesToSystemMem();
			RenderResource::LostAllResources();

			ResizeView(w, h);

			RenderManager::Instance()->Invalidate();
			RenderResource::InvalidateAllResources();
			SceneCache::Instance()->InvalidateSceneMaterials();
        }
		else
		{
			wasCreated = true;

			Logger::Debug("[CorePlatformAndroid::] before create renderer");
			const char* glVersion = reinterpret_cast<const char*>(glGetString(GL_VERSION));
			Logger::Debug("RENDERER glVersion %s", glVersion);
			if (nullptr != glVersion)
			{
				String ver(glVersion);
				auto found = ver.find_first_of(".");
				if (found != String::npos && found > 0)
				{
					char cv = ver.at(found-1);
					int major = cv - '0';
					if(major >= 3)
					{
						RenderManager::Create(Core::RENDERER_OPENGL_ES_3_0);
						Logger::Debug("RENDERER_OPENGL_ES_3_0 ");
					} else
					{
						RenderManager::Create(Core::RENDERER_OPENGL_ES_2_0);
						Logger::Debug("RENDERER_OPENGL_ES_2_0 ");
					}
				}else
				{
					RenderManager::Create(Core::RENDERER_OPENGL_ES_2_0);
					Logger::Debug("RENDERER_OPENGL_ES_2_0 GLVersion invalid format");
				}

			} else
			{
				RenderManager::Create(Core::RENDERER_OPENGL_ES_2_0);
				Logger::Debug("RENDERER_OPENGL_ES_2_0 NULL");
			}

			FileSystem::Instance()->Init();
			RenderSystem2D::Instance()->Init();

			RenderManager::Instance()->InitFBO(androidDelegate->RenderBuffer(), androidDelegate->FrameBuffer());
			Logger::Debug("[CorePlatformAndroid::] after create renderer");

			ResizeView(w, h);
			// Set proper width and height before call FrameworkDidlaunched
			FrameworkDidLaunched();

			RenderManager::Instance()->SetFPS(60);

			//////////////////////////////////////////////////////////////////////////
			Core::Instance()->SystemAppStarted();

			StartForeground();
		}

		Logger::Debug("[CorePlatformAndroid::RenderRecreated] end");
	}

	void CorePlatformAndroid::OnCreateActivity()
	{
//		Logger::Debug("[CorePlatformAndroid::OnCreateActivity]");
	}

	void CorePlatformAndroid::OnDestroyActivity()
	{
//		Logger::Debug("[CorePlatformAndroid::OnDestroyActivity]");

		renderIsActive = false;
	}

	void CorePlatformAndroid::StartVisible()
	{
//		Logger::Debug("[CorePlatformAndroid::StartVisible]");
	}

	void CorePlatformAndroid::StopVisible()
	{
//		Logger::Debug("[CorePlatformAndroid::StopVisible]");
	}

	void CorePlatformAndroid::StartForeground()
	{
		Logger::Debug("[CorePlatformAndroid::StartForeground] start");

		if(wasCreated)
		{
			DAVA::ApplicationCore * core = DAVA::Core::Instance()->GetApplicationCore();
			if(core)
			{
				core->OnResume();
			}
			else
			{
				DAVA::Core::Instance()->SetIsActive(true);
			}
			DAVA::Core::Instance()->GoForeground();

			foreground = true;
		}
		Logger::Debug("[CorePlatformAndroid::StartForeground] end");
	}

	void CorePlatformAndroid::StopForeground(bool isLock)
	{
		Logger::Debug("[CorePlatformAndroid::StopForeground]");

		DAVA::ApplicationCore * core = DAVA::Core::Instance()->GetApplicationCore();
		if(core)
		{
			core->OnSuspend();
		}
		else
		{
			DAVA::Core::Instance()->SetIsActive(false);
		}
		DAVA::Core::Instance()->GoBackground(isLock);

		foreground = false;
	}

	void CorePlatformAndroid::KeyUp(int32 keyCode)
	{
		InputSystem::Instance()->GetKeyboard().OnSystemKeyUnpressed(keyCode);
	}

	void CorePlatformAndroid::KeyDown(int32 keyCode)
	{
		InputSystem::Instance()->GetKeyboard().OnSystemKeyPressed(keyCode);

		UIEvent * keyEvent = new UIEvent;
		keyEvent->keyChar = 0;
		keyEvent->phase = DAVA::UIEvent::PHASE_KEYCHAR;
		keyEvent->tapCount = 1;
		keyEvent->tid = InputSystem::Instance()->GetKeyboard().GetDavaKeyForSystemKey(keyCode);

		InputSystem::Instance()->ProcessInputEvent(keyEvent);

		SafeDelete(keyEvent);
	}

	void CorePlatformAndroid::OnGamepadElement(int32 elementKey, float32 value, bool isKeycode)
	{
		GamepadDevice & gamepadDevice = InputSystem::Instance()->GetGamepadDevice();

		int32 davaKey = GamepadDevice::INVALID_DAVAKEY;
		if(isKeycode)
			davaKey = gamepadDevice.GetDavaEventIdForSystemKeycode(elementKey);
		else
			davaKey = gamepadDevice.GetDavaEventIdForSystemAxis(elementKey);

		if(davaKey == GamepadDevice::INVALID_DAVAKEY)
			return;

		UIEvent newEvent;
		newEvent.tid = davaKey;
		newEvent.physPoint.x = value;
		newEvent.point.x = value;
		newEvent.phase = DAVA::UIEvent::PHASE_JOYSTICK;

		gamepadDevice.SystemProcessElement(static_cast<GamepadDevice::eDavaGamepadElement>(davaKey), value);
		InputSystem::Instance()->ProcessInputEvent(&newEvent);
	}

	void CorePlatformAndroid::OnGamepadAvailable(bool isAvailable)
	{
		InputSystem::Instance()->GetGamepadDevice().SetAvailable(isAvailable);
	}

	void CorePlatformAndroid::OnGamepadTriggersAvailable(bool isAvailable)
	{
		InputSystem::Instance()->GetGamepadDevice().OnTriggersAvailable(isAvailable);
	}

	void CorePlatformAndroid::OnInput(int32 action, int32 source, Vector<UIEvent>& activeInputs, Vector<UIEvent>& allInputs)
	{
		DVASSERT(!allInputs.empty());
		if (!allInputs.empty())
		{
			UIControlSystem::Instance()->OnInput(action, activeInputs, allInputs, allInputs[0].timestamp);
		}
	}

    bool CorePlatformAndroid::IsMultitouchEnabled()
    {
        return InputSystem::Instance()->GetMultitouchEnabled();
    }
    
	bool CorePlatformAndroid::DownloadHttpFile(const String & url, const String & documentsPathname)
	{
		if(androidDelegate)
		{
			FilePath path(documentsPathname);
			return androidDelegate->DownloadHttpFile(url, path.GetAbsolutePathname());
		}

		return false;
	}

	AndroidSystemDelegate* CorePlatformAndroid::GetAndroidSystemDelegate() const
	{
		return androidDelegate;
	}
}
#endif // #if defined(__DAVAENGINE_ANDROID__)
