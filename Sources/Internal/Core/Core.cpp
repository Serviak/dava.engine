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

#include "DAVAClassRegistrator.h"

#include "FileSystem/FileSystem.h"
#include "Base/ObjectFactory.h"
#include "Core/ApplicationCore.h"
#include "Core/Core.h"
#include "Core/PerformanceSettings.h"
#include "Render/RenderManager.h"
#include "Platform/SystemTimer.h"
#include "Platform/Thread.h"
#include "UI/UIScreenManager.h"
#include "UI/UIControlSystem.h"
#include "Input/InputSystem.h"
#include "Debug/DVAssert.h"
#include "Render/2D/TextBlock.h"
#include "Debug/Replay.h"
#include "Sound/SoundSystem.h"
#include "Sound/SoundEvent.h"
#include "Input/InputSystem.h"
#include "Platform/DPIHelper.h"
#include "Base/AllocatorFactory.h"
#include "Render/2D/FTFont.h"
#include "Scene3D/SceneFile/VersionInfo.h"
#include "Render/Image/ImageSystem.h"
#include "Scene3D/SceneCache.h"
#include "Render/2D/Systems/VirtualCoordinatesSystem.h"
#include "Render/2D/Systems/RenderSystem2D.h"
#include "DLC/Downloader/DownloadManager.h"
#include "DLC/Downloader/CurlDownloader.h"
#include "Render/OcclusionQuery.h"
#include "Notification/LocalNotificationController.h"
#include "Platform/DeviceInfo.h"

#include "Network/NetCore.h"

#if defined(__DAVAENGINE_ANDROID__)
#include "Platform/TemplateAndroid/AssetsManagerAndroid.h"
#endif

#if defined(__DAVAENGINE_IPHONE__)
// not used
#elif defined(__DAVAENGINE_ANDROID__)
#	include "Input/AccelerometerAndroid.h"
#endif //PLATFORMS

#ifdef __DAVAENGINE_NVIDIA_TEGRA_PROFILE__
#include <EGL/eglext.h>
#endif //__DAVAENGINE_NVIDIA_TEGRA_PROFILE__

#ifdef __DAVAENGINE_AUTOTESTING__
#include "Autotesting/AutotestingSystem.h"
#endif


namespace DAVA 
{

#if defined(__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_ANDROID__)
	static bool useAutodetectContentScaleFactor = false;
#endif //#if defined(__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_ANDROID__)
    
static ApplicationCore * core = nullptr;

Core::Core()
{
    globalFrameIndex = 1;
    isActive = false;
    firstRun = true;
	isConsoleMode = false;
	options = new KeyedArchive();
}

Core::~Core()
{
	SafeRelease(options);
	SafeRelease(core);
}


void Core::CreateSingletons()
{
    // check types size
	new Logger();
	new AllocatorFactory();
	new JobManager();
	new FileSystem();
    FilePath::InitializeBundleName();
	
	FileSystem::Instance()->SetDefaultDocumentsDirectory();
    FileSystem::Instance()->CreateDirectory(FileSystem::Instance()->GetCurrentDocumentsDirectory(), true);
	
    new SoundSystem();

	if (isConsoleMode)
	{
		/*
			Disable all debug initialization messages in console mode
		 */
		Logger::Instance()->SetLogLevel(Logger::LEVEL_INFO);
	}
    
	new LocalizationSystem();

	new SystemTimer();
	new Random();
	new AnimationManager();
	new FontManager();
	new UIControlSystem();
	new InputSystem();
	new RenderHelper();
    new RenderLayerManager();
	new PerformanceSettings();
    new VersionInfo();
    new ImageSystem();
    new SceneCache();
    new FrameOcclusionQueryManager();
	
    new VirtualCoordinatesSystem();
    new RenderSystem2D();

#if defined(__DAVAENGINE_ANDROID__)
    new AssetsManager();
#endif
	
#if defined __DAVAENGINE_IPHONE__
	// not used
#elif defined(__DAVAENGINE_ANDROID__)
	new AccelerometerAndroidImpl();
#endif //#if defined __DAVAENGINE_IPHONE__
	
	new UIScreenManager();

	Thread::InitMainThread();

    new DownloadManager();
    DownloadManager::Instance()->SetDownloader(new CurlDownloader());
    
    new LocalNotificationController();

    DeviceInfo::InitializeScreenInfo();
    
    RegisterDAVAClasses();
    CheckDataTypeSizes();

    new Net::NetCore();

#ifdef __DAVAENGINE_AUTOTESTING__
	new AutotestingSystem();
#endif
}

// We do not create RenderManager until we know which version of render manager we want to create
void Core::CreateRenderManager()
{
    eRenderer renderer = (eRenderer)options->GetInt32("renderer");
    
    RenderManager::Create(renderer);	
}
        
void Core::ReleaseSingletons()
{
    // Finish network infrastructure
    // As I/O event loop runs in main thread so NetCore should run out loop to make graceful shutdown
    Net::NetCore::Instance()->Finish(true);
    Net::NetCore::Instance()->Release();

#ifdef __DAVAENGINE_AUTOTESTING__
	AutotestingSystem::Instance()->Release();
#endif

	LocalNotificationController::Instance()->Release();
    DownloadManager::Instance()->Release();
	PerformanceSettings::Instance()->Release();
	RenderHelper::Instance()->Release();
	UIScreenManager::Instance()->Release();
	UIControlSystem::Instance()->Release();
	FontManager::Instance()->Release();
	AnimationManager::Instance()->Release();
	SystemTimer::Instance()->Release();
#if defined(__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_ANDROID__)
	Accelerometer::Instance()->Release();
	//SoundSystem::Instance()->Release();
#endif //#if defined(__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_ANDROID__)
	LocalizationSystem::Instance()->Release();
//	Logger::FrameworkDebug("[Core::Release] successfull");
    FileSystem::Instance()->Release();
    SoundSystem::Instance()->Release();
	Random::Instance()->Release();
	RenderLayerManager::Instance()->Release();
    FrameOcclusionQueryManager::Instance()->Release();
    VirtualCoordinatesSystem::Instance()->Release();
    RenderSystem2D::Instance()->Release();
	RenderManager::Instance()->Release();

	InputSystem::Instance()->Release();
	JobManager::Instance()->Release();
    VersionInfo::Instance()->Release();
	AllocatorFactory::Instance()->Release();
	Logger::Instance()->Release();
    ImageSystem::Instance()->Release();
    SceneCache::Instance()->Release();

#if defined(__DAVAENGINE_ANDROID__)
    AssetsManager::Instance()->Release();
#endif
}

void Core::SetOptions(KeyedArchive * archiveOfOptions)
{
	SafeRelease(options);

	options = SafeRetain(archiveOfOptions);
#if defined(__DAVAENGINE_IPHONE__)
		useAutodetectContentScaleFactor = options->GetBool("iPhone_autodetectScreenScaleFactor", false);
#elif defined(__DAVAENGINE_ANDROID__)
		useAutodetectContentScaleFactor = options->GetBool("Android_autodetectScreenScaleFactor", false);
#endif //#if defined(__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_ANDROID__)

#if !defined(__DAVAENGINE_ANDROID__)
	//YZ android platform always use SCREEN_ORIENTATION_PORTRAIT and rotate system view and don't rotate GL view  
	screenOrientation = options->GetInt32("orientation", SCREEN_ORIENTATION_PORTRAIT);
#endif
}
    
void Core::CheckDataTypeSizes()
{
    CheckType(int8(), 8, "int8");
    CheckType(uint8(), 8, "uint8");
    CheckType(int16(), 16, "int16");
    CheckType(uint16(), 16, "uint16");
    CheckType(int32(), 32, "int32");
    CheckType(uint32(), 32, "uint32");
}

template <class T> void Core::CheckType(T t, int32 expectedSize, const char * typeString)
{
    if ((sizeof(t) * 8) != expectedSize)
    {
        Logger::Error("Size of %s is incorrect. Expected size: %d. Platform size: %d", typeString, expectedSize, sizeof(t));
    }
}

KeyedArchive * Core::GetOptions()
{
	return options;
}
	
#if defined(__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_ANDROID__)
bool Core::IsAutodetectContentScaleFactor()
{
	return useAutodetectContentScaleFactor;
}
#endif //#if defined(__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_ANDROID__)

Core::eScreenOrientation Core::GetScreenOrientation()
{
	return (Core::eScreenOrientation)screenOrientation;
}
	
Core::eScreenMode Core::GetScreenMode()
{
	Logger::FrameworkDebug("[Core::GetScreenMode] return screen mode MODE_UNSUPPORTED");
	return MODE_UNSUPPORTED;
}

void Core::SwitchScreenToMode(eScreenMode screenMode)
{
	Logger::FrameworkDebug("[Core::SwitchScreenToMode] do not supported by platform implementation of core");
}

void Core::GetAvailableDisplayModes(List<DisplayMode> & availableModes)
{	

}
void Core::ToggleFullscreen()
{
	
}

DisplayMode Core::FindBestMode(const DisplayMode & requestedMode)
{
	List<DisplayMode> availableDisplayModes;
	GetAvailableDisplayModes(availableDisplayModes);

	DisplayMode bestMatchMode;

	bestMatchMode.refreshRate = -1;
	for (List<DisplayMode>::iterator it = availableDisplayModes.begin(); it != availableDisplayModes.end(); ++it)
	{
		DisplayMode & availableMode = *it;
		if ((availableMode.width == requestedMode.width) && (availableMode.height == requestedMode.height))
		{
			// if first mode found replace
			if (bestMatchMode.refreshRate == -1)
				bestMatchMode = availableMode;

			if (availableMode.bpp > bestMatchMode.bpp) // find best match with highest bits per pixel
			{
				bestMatchMode = availableMode;
			}
		}
	}

	if (bestMatchMode.refreshRate == -1) // haven't found any mode
	{
		int32 minDiffWidth = 0;
		int32 minDiffHeight = 0;
		float32 requestedAspect = (requestedMode.height>0?(float32)requestedMode.width/(float32)requestedMode.height:1.0f);
		float32 minDiffAspect = 0;

		for (List<DisplayMode>::iterator it = availableDisplayModes.begin(); it != availableDisplayModes.end(); ++it)
		{
			DisplayMode & availableMode = *it;

			int32 diffWidth = abs(availableMode.width - requestedMode.width);
			int32 diffHeight = abs(availableMode.height - requestedMode.height);

			float32 availableAspect = (availableMode.height>0?(float32)availableMode.width/(float32)availableMode.height:1.0f);
			float32 diffAspect = fabsf(availableAspect - requestedAspect);

//			if (diffWidth >= 0 && diffHeight >= 0)
			{
				// if first mode found replace
				if (bestMatchMode.refreshRate == -1)
				{
					minDiffWidth = diffWidth;
					minDiffHeight = diffHeight;
					minDiffAspect = diffAspect;
				}

				if(diffAspect<=(minDiffAspect+0.01f))
				{
					if((diffAspect+0.01f)<minDiffAspect)
					{
						// aspect changed, clear min diff
						minDiffWidth = diffWidth;
						minDiffHeight = diffHeight;
					}

					minDiffAspect = diffAspect;

					//int32 curDiffWidth = availableMode.width - bestMatchMode.width;
					//int32 curDiffHeight = availableMode.height - bestMatchMode.height;

					//if (diffWidth + diffHeight <= curDiffWidth + curDiffHeight)
					if (diffWidth + diffHeight <= minDiffWidth + minDiffHeight)
					{
						minDiffWidth = diffWidth;
						minDiffHeight = diffHeight;

						if (availableMode.bpp >= bestMatchMode.bpp) // find best match with highest bits per pixel
						{
							bestMatchMode = availableMode;
						}
					}
				}
			}
		}
	}

	if (bestMatchMode.refreshRate == -1) // haven't found any mode
	{
		int maxRes = 0;
		for (List<DisplayMode>::iterator it = availableDisplayModes.begin(); it != availableDisplayModes.end(); ++it)
		{
			DisplayMode & availableMode = *it;

			//int32 diffWidth = availableMode.width ;
			//int32 diffHeight = availableMode.height - requestedMode.height;
			if (availableMode.width + availableMode.height + availableMode.bpp > maxRes)
			{
				maxRes = availableMode.width + availableMode.height + availableMode.bpp;
				bestMatchMode = availableMode;
			}
		}
	}
	return bestMatchMode;
}

DisplayMode Core::GetCurrentDisplayMode()
{
	return DisplayMode();
}

void Core::Quit()
{
    exit(0);
	Logger::FrameworkDebug("[Core::Quit] do not supported by platform implementation of core");
}
	
void Core::SetApplicationCore(ApplicationCore * _core)
{
	core = _core;
}

ApplicationCore * Core::GetApplicationCore()
{
	return core;
}
	
void Core::SystemAppStarted()
{
	if (VirtualCoordinatesSystem::Instance()->WasScreenSizeChanged())
	{
		VirtualCoordinatesSystem::Instance()->ScreenSizeChanged();
		/*  Question to Hottych: Does it really necessary here?
            RenderManager::Instance()->SetRenderOrientation(Core::Instance()->GetScreenOrientation());
         */
	}

	if (core != nullptr)
	{
        core->OnAppStarted();
    }
}
	
void Core::SystemAppFinished()
{
    if (core != nullptr)
    {
        core->OnAppFinished();
    }
}


void Core::SystemProcessFrame()
{
#ifdef __DAVAENGINE_NVIDIA_TEGRA_PROFILE__
	static bool isInit = false;
	static EGLuint64NV frequency;
	static PFNEGLGETSYSTEMTIMENVPROC eglGetSystemTimeNV;
	static PFNEGLGETSYSTEMTIMEFREQUENCYNVPROC eglGetSystemTimeFrequencyNV;
	if (!isInit)
	{
		eglGetSystemTimeNV = (PFNEGLGETSYSTEMTIMENVPROC) eglGetProcAddress("eglGetSystemTimeNV");
		eglGetSystemTimeFrequencyNV = (PFNEGLGETSYSTEMTIMEFREQUENCYNVPROC) eglGetProcAddress("eglGetSystemTimeFrequencyNV");
		if (!eglGetSystemTimeNV || !eglGetSystemTimeFrequencyNV)
		{
			DVASSERT(!"Error export eglGetSystemTimeNV, eglGetSystemTimeFrequencyNV");
			exit(0);
		}
		frequency = eglGetSystemTimeFrequencyNV();
	}
	EGLuint64NV start = eglGetSystemTimeNV() / frequency;
#endif //__DAVAENGINE_NVIDIA_TEGRA_PROFILE__
    Stats::Instance()->BeginFrame();
    TIME_PROFILE("Core::SystemProcessFrame");
    
    // Poll for network I/O events here, not depending on Core active flag
    Net::NetCore::Instance()->Poll();
    
	if (!core) return;
	if (!isActive)return;
	
	SystemTimer::Instance()->Start();

	/**
		Check if device not in lost state first / after that be
	*/
	if (!RenderManager::Instance()->IsDeviceLost())
	{
// #ifdef __DAVAENGINE_DIRECTX9__
// 		if(firstRun)
// 		{
// 			core->BeginFrame();
// 			firstRun = false;
// 		}
// #else
        InputSystem::Instance()->OnBeforeUpdate();
		core->BeginFrame();
//#endif

#if !defined(__DAVAENGINE_ANDROID__)
		RenderResource::SaveAllResourcesToSystemMem();
#endif //#if !defined(__DAVAENGINE_ANDROID__)

		// recalc frame inside begin / end frame
		if (VirtualCoordinatesSystem::Instance()->WasScreenSizeChanged())
		{
			VirtualCoordinatesSystem::Instance()->ScreenSizeChanged();
            UIScreenManager::Instance()->ScreenSizeChanged();
            UIControlSystem::Instance()->ScreenSizeChanged();
		}

		float32 frameDelta = SystemTimer::Instance()->FrameDelta();
        SystemTimer::Instance()->UpdateGlobalTime(frameDelta);

		if(Replay::IsRecord())
		{
			Replay::Instance()->RecordFrame(frameDelta);
		}
		if(Replay::IsPlayback())
		{
			UIControlSystem::Instance()->ReplayEvents();
			frameDelta = Replay::Instance()->PlayFrameTime();
			if(Replay::IsPlayback()) //can be unset in previous string
			{
				SystemTimer::Instance()->SetFrameDelta(frameDelta);
			}
		}
		
		LocalNotificationController::Instance()->Update();
        DownloadManager::Instance()->Update();
		JobManager::Instance()->Update();

		core->Update(frameDelta);
        InputSystem::Instance()->OnAfterUpdate();
		core->Draw();

		core->EndFrame();
// #ifdef __DAVAENGINE_DIRECTX9__
// 		core->BeginFrame();
// #endif
	}
    Stats::Instance()->EndFrame();
	globalFrameIndex++;
	
#ifdef __DAVAENGINE_NVIDIA_TEGRA_PROFILE__
	EGLuint64NV end = eglGetSystemTimeNV() / frequency;
	EGLuint64NV interval = end - start;
#endif //__DAVAENGINE_NVIDIA_TEGRA_PROFILE__
}

	
void Core::GoBackground(bool isLock)
{
#if defined (__DAVAENGINE_IPHONE__) || defined (__DAVAENGINE_ANDROID__) 
	if (core)
    {
        if(isLock)
        {
            core->OnDeviceLocked();
        }
        else
        {
            core->OnBackground();
        }
    }
#endif //#if defined (__DAVAENGINE_IPHONE__) || defined (__DAVAENGINE_ANDROID__)
}

void Core::GoForeground()
{
#if defined (__DAVAENGINE_IPHONE__) || defined (__DAVAENGINE_ANDROID__)
	if (core)
	{
		core->OnForeground();
	}
    Net::NetCore::Instance()->RestartAllControllers();
#endif //#if defined (__DAVAENGINE_IPHONE__) || defined (__DAVAENGINE_ANDROID__)
}

uint32 Core::GetGlobalFrameIndex()
{
	return globalFrameIndex;
}
	
void Core::SetCommandLine(int argc, char *argv[])
{
    commandLine.assign(argv, argv + argc);
}

void Core::SetCommandLine(const DAVA::String& cmdLine)
{
    commandLine.clear();
    Split(cmdLine, " ", commandLine);
}

Vector<String> & Core::GetCommandLine()
{
	return commandLine;
}
	
bool Core::IsConsoleMode()
{
	return isConsoleMode;
}
	
void Core::EnableConsoleMode()
{
	isConsoleMode = true;
}

void Core::SetIsActive(bool _isActive)
{
	isActive = _isActive;
}

#if defined (__DAVAENGINE_MACOS__) || defined (__DAVAENGINE_WIN32__)    
Core::eDeviceFamily Core::GetDeviceFamily()
{
    return DEVICE_DESKTOP;
}
#endif //#if defined (__DAVAENGINE_MACOS__) || defined (__DAVAENGINE_WIN32__)
    
uint32 Core::GetScreenDPI()
{
	return DPIHelper::GetScreenDPI();
}

void Core::SetIcon(int32 /*iconId*/)
{
};

};

