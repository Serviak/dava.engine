/*
 *  CorePlatformAndroid.cpp
 *  TemplateProjectAndroid
 *
 *  Created by Viktor  Kleschenko on 2/18/11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */
#include "Platform/TemplateAndroid/CorePlatformAndroid.h"
#include "Platform/TemplateAndroid/AndroidSpecifics.h"

//#include "Core/Core.h"

#if defined(__DAVAENGINE_ANDROID__)

extern void FrameworkDidLaunched();
extern void FrameworkWillTerminate();

#include "Platform/Thread.h"

namespace DAVA
{

	Core::eDeviceFamily Core::GetDeviceFamily()
	{
		//TODO: need to create real function
		return DEVICE_HANDSET;
	}

	CorePlatformAndroid::CorePlatformAndroid()
	: Core()
	{
//		Logger::Debug("[CorePlatformAndroid::CorePlatformAndroid]");

		wasCreated = false;
		renderIsActive = false;
		oldWidth = 0;
		oldHeight = 0;

		foreground = false;
	}

	int Core::Run(int argc, char * argv[], AppHandle handle)
	{
// 		CoreWin32Platform * core = new CoreWin32Platform();
// 		core->CreateWin32Window(handle);
		// 		core->Run();
// 		core->ReleaseSingletons();
// #ifdef ENABLE_MEMORY_MANAGER
// 		if (DAVA::MemoryManager::Instance() != 0)
// 		{
// 			DAVA::MemoryManager::Instance()->FinalLog();
// 		}
// #endif
		return 0;

	}

	void CorePlatformAndroid::Quit()
	{
		Logger::Debug("[CorePlatformAndroid::Quit]");
		QuitAction();
	}

	void CorePlatformAndroid::QuitAction()
	{
		Logger::Debug("[CorePlatformAndroid::QuitAction]");

		if(Core::Instance())
		{
			Core::Instance()->SystemAppFinished();
		}

		FrameworkWillTerminate();

#ifdef ENABLE_MEMORY_MANAGER
		if (DAVA::MemoryManager::Instance() != 0)
		{
			DAVA::MemoryManager::Instance()->FinalLog();
		}
#endif

		Logger::Debug("[CorePlatformAndroid::QuitAction] done");
	}

	void CorePlatformAndroid::RepaintView()
	{
		if(renderIsActive)
		{
			DAVA::RenderManager::Instance()->Lock();
			Core::SystemProcessFrame();
			DAVA::RenderManager::Instance()->Unlock();
		}
	}

	void CorePlatformAndroid::ResizeView(int32 w, int32 h)
	{
		if(oldWidth != w || oldHeight != h)
		{
			oldWidth = w;
			oldHeight = h;

			windowedMode.width = w;
			windowedMode.height = h;

			UpdateScreenMode();
		}
	}

	void CorePlatformAndroid::UpdateScreenMode()
	{
		Logger::Debug("[CorePlatformAndroid::UpdateScreenMode] start");
		UIControlSystem::Instance()->SetInputScreenAreaSize(windowedMode.width, windowedMode.height);
		Core::Instance()->SetPhysicalScreenSize(windowedMode.width, windowedMode.height);

		RenderManager::Instance()->InitFBSize(windowedMode.width, windowedMode.height);
        RenderManager::Instance()->Init(windowedMode.width, windowedMode.height);

		Logger::Debug("[CorePlatformAndroid::] w = %d, h = %d", windowedMode.width, windowedMode.height);
		Logger::Debug("[CorePlatformAndroid::UpdateScreenMode] done");
	}

	void CorePlatformAndroid::CreateAndroidWindow(const char8 *docPath, const char8 *assets, const char8 *logTag, AndroidSystemDelegate * sysDelegate)
	{
		androidDelegate = sysDelegate;

		Core::CreateSingletons();

		Logger::SetTag(logTag);
//		Logger::Debug("[CorePlatformAndroid::CreateAndroidWindow] docpath = %s", docPath);
//		Logger::Debug("[CorePlatformAndroid::CreateAndroidWindow] assets = %s", assets);

		//FileSystem::Instance()->SetPath(docPath, assets);

		//////////////////////////////////////////////////////////////////////////
		windowedMode = DisplayMode(480, 320, 16, 0);
	}

	void CorePlatformAndroid::RenderRecreated()
	{
		Logger::Debug("[CorePlatformAndroid::RenderRecreated] start");

		renderIsActive = true;

		Thread::InitMainThread();

		if(wasCreated)
		{
			RenderResource::InvalidateAllResources();
		}
		else
		{
			wasCreated = true;

			Logger::Debug("[CorePlatformAndroid::] before create renderer");
			RenderManager::Create(Core::RENDERER_OPENGL_ES_2_0);

			RenderManager::Instance()->InitFBO(androidDelegate->RenderBuffer(), androidDelegate->FrameBuffer());
			Logger::Debug("[CorePlatformAndroid::] after create renderer");

			FrameworkDidLaunched();
			screenOrientation = Core::SCREEN_ORIENTATION_PORTRAIT; //no need rotate GL we on Android

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
		//TODO: VK: add code for handling

		if(wasCreated)
		{
			ApplicationCore * core = Core::Instance()->GetApplicationCore();
			if(core)
			{
				core->OnResume();
			}

			foreground = true;
		}
		Logger::Debug("[CorePlatformAndroid::StartForeground] end");
	}

	void CorePlatformAndroid::StopForeground()
	{
		Logger::Debug("[CorePlatformAndroid::StopForeground]");
		//TODO: VK: add code for handling

		RenderResource::SaveAllResourcesToSystemMem();
		RenderResource::LostAllResources();

		ApplicationCore * core = Core::Instance()->GetApplicationCore();
		if(core)
		{
			core->OnSuspend();
		}

		foreground = false;

		oldWidth = 0;
		oldHeight = 0;
	}

	static Vector<DAVA::UIEvent> activeTouches;
	void CorePlatformAndroid::KeyUp(int32 keyCode)
	{
		Vector<DAVA::UIEvent> touches;
		Vector<DAVA::UIEvent> emptyTouches;

		for(Vector<DAVA::UIEvent>::iterator it = activeTouches.begin(); it != activeTouches.end(); it++)
		{
			touches.push_back(*it);
		}

		DAVA::UIEvent ev;
		ev.keyChar = (char16)keyCode;
		ev.phase = DAVA::UIEvent::PHASE_KEYCHAR;
		ev.tapCount = 1;
		ev.tid = (int32)keyCode;

		touches.push_back(ev);

		UIControlSystem::Instance()->OnInput(0, emptyTouches, touches);
		touches.pop_back();
		UIControlSystem::Instance()->OnInput(0, emptyTouches, touches);
	}

	void CorePlatformAndroid::KeyDown(int32 keyCode)
	{
	}

	UIEvent CorePlatformAndroid::CreateTouchEvent(int32 action, int32 id, float32 x, float32 y, long time)
	{
		int32 phase = DAVA::UIEvent::PHASE_DRAG;
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
			phase = DAVA::UIEvent::PHASE_DRAG;
			break;

			case 3://ACTION_CANCEL
			phase = DAVA::UIEvent::PHASE_CANCELLED;
			break;

			case 4://ACTION_OUTSIDE
			break;
		}

		UIEvent newTouch;
		newTouch.tid = id;
		newTouch.physPoint.x = x;
		newTouch.physPoint.y = y;
		newTouch.phase = phase;
		newTouch.tapCount = 1;

		return newTouch;
	}
	AAssetManager * CorePlatformAndroid::GetAssetManager()
	{
		return assetMngr;
	}
	void CorePlatformAndroid::SetAssetManager(AAssetManager * mngr)
	{
		assetMngr = mngr;
	}
	void CorePlatformAndroid::OnTouch(int32 action, int32 id, float32 x, float32 y, long time)
	{
//		Logger::Debug("[CorePlatformAndroid::OnTouch] IN totalTouches.size = %d", totalTouches.size());
//		Logger::Debug("[CorePlatformAndroid::OnTouch] action is %d, id is %d, x is %f, y is %f, time is %d", action, id, x, y, time);

		UIEvent touchEvent = CreateTouchEvent(action, id, x, y, time);
		Vector<DAVA::UIEvent> activeTouches;
		activeTouches.push_back(touchEvent);

		bool isFound = false;
		for(Vector<DAVA::UIEvent>::iterator it = totalTouches.begin(); it != totalTouches.end(); ++it)
		{
			if((*it).tid == touchEvent.tid)
			{
				(*it).physPoint.x = touchEvent.physPoint.x;
				(*it).physPoint.y = touchEvent.physPoint.y;
				(*it).phase = touchEvent.phase;
				(*it).tapCount = touchEvent.tapCount;

				isFound = true;
			}
		}
		if(!isFound)
		{
			totalTouches.push_back(touchEvent);
		}

		UIControlSystem::Instance()->OnInput(touchEvent.phase, activeTouches, totalTouches);

		for(Vector<DAVA::UIEvent>::iterator it = totalTouches.begin(); it != totalTouches.end(); )
		{
			if((DAVA::UIEvent::PHASE_ENDED == (*it).phase) || (DAVA::UIEvent::PHASE_CANCELLED == (*it).phase))
			{
				it = totalTouches.erase(it);
			}
			else
			{
				++it;
			}
		}
//		Logger::Debug("[CorePlatformAndroid::OnTouch] OUT totalTouches.size = %d", totalTouches.size());
	}

	bool CorePlatformAndroid::DownloadHttpFile(const String & url, const String & documentsPathname)
	{
		if(androidDelegate)
		{
			String path = FileSystem::Instance()->SystemPathForFrameworkPath(documentsPathname);
			return androidDelegate->DownloadHttpFile(url, path);
		}

		return false;
	}
}
#endif // #if defined(__DAVAENGINE_ANDROID__)
