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
#include "Platform/Qt/Win32/CorePlatformWin32.h"
#include "Platform/Qt/Win32/WindowsSpecifics.h"
#include "Platform/Thread.h"
#include "Utils/Utils.h"

#if defined(__DAVAENGINE_WIN32__)

#include <shellapi.h>

extern void FrameworkDidLaunched();
extern void FrameworkWillTerminate();

namespace DAVA 
{
	int Core::Run(int argc, char * argv[], AppHandle handle)
	{
		CoreWin32Platform * core = new CoreWin32Platform();
		core->CreateSingletons();
		core->InitArgs();

// 		bool windowCreated = core->CreateWin32Window(handle);
// 		if(windowCreated)
// 		{
//			core->Run();
// 			core->ReleaseSingletons();
// 			
// #ifdef ENABLE_MEMORY_MANAGER
// 			if (DAVA::MemoryManager::Instance() != 0)
// 			{
// 				DAVA::MemoryManager::Instance()->FinalLog();
// 			}
// #endif
// 		}
// 
// 		CloseHandle(core->hMutex);

		return 0;
	}

	/*const Vector2 & Core::GetMouseLocation();
	{
		POINT pt;
		GetCursorPos(&pt);
		
	}*/
    
	int Core::RunCmdTool(int argc, char * argv[], AppHandle handle)
	{
		CoreWin32Platform * core = new CoreWin32Platform();

		//core->CreateWin32Window(handle);
		//core->Run();
		core->EnableConsoleMode();
		core->CreateSingletons();

		core->InitArgs();

		FrameworkDidLaunched();
		FrameworkWillTerminate();
		core->ReleaseSingletons();
#ifdef ENABLE_MEMORY_MANAGER
		if (DAVA::MemoryManager::Instance() != 0)
		{
			DAVA::MemoryManager::Instance()->FinalLog();
		}
#endif
		return 0;

	}
	
	void CoreWin32Platform::InitArgs()
	{
		LPWSTR *szArglist;
		int nArgs;
		int i;
		szArglist = ::CommandLineToArgvW(::GetCommandLineW(), &nArgs);
		if( NULL == szArglist )
		{
			Logger::Error("CommandLineToArgvW failed\n");
			return;
		}
		else 
		{
			Vector<String> & cl = GetCommandLine();
			for( i=0; i<nArgs; i++)
			{
				WideString w = szArglist[i];
				String nonWide = WStringToString(w);
				cl.push_back(nonWide);
				Logger::Debug("%d: %s\n", i, nonWide.c_str());
			}
		}
		// Free memory allocated for CommandLineToArgvW arguments.
		LocalFree(szArglist);
	}

//	LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

// 	bool CoreWin32Platform::CreateWin32Window(HINSTANCE hInstance)
// 	{	
// 		//single instance check
// 		TCHAR fileName[MAX_PATH];
// 		GetModuleFileName(NULL, fileName, MAX_PATH);
// 		fileName[MAX_PATH-1] = 0; //string can be not null-terminated on winXP
// 		for(int32 i = 0; i < MAX_PATH; ++i)
// 		{
// 			if(fileName[i] == L'\\') //symbol \ is not allowed in CreateMutex mutex name
// 			{
// 				fileName[i] = ' ';
// 			}
// 		}
// 		hMutex = CreateMutex(NULL, FALSE, fileName);
// 		if(ERROR_ALREADY_EXISTS == GetLastError())
// 		{
// 			return false;
// 		}
// 
// 		windowedMode = DisplayMode(800, 600, 16, 0);
// 		fullscreenMode = DisplayMode(800, 600, 16, 0);
// 		currentMode = windowedMode;
// 		isFullscreen = false;
// 
// 		// create the window, only if we do not use the null device
// 		LPCWSTR className = L"DavaFrameworkWindowsDevice";
// 
// 		// Register Class
// 
// 		WNDCLASSEX wcex;
// 		wcex.cbSize = sizeof(WNDCLASSEX); 
// 		wcex.style			= CS_BYTEALIGNCLIENT | CS_HREDRAW | CS_VREDRAW;
// 		wcex.lpfnWndProc	= (WNDPROC)WndProc;
// 		wcex.cbClsExtra		= 0;
// 		wcex.cbWndExtra		= 0;
// 		wcex.hInstance		= hInstance;
// 		wcex.hIcon			= 0;
// 		wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
// 		wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
// 		wcex.lpszMenuName	= 0;
// 		wcex.lpszClassName	= className;
// 		wcex.hIconSm		= 0;
// 
// 		RegisterClassEx(&wcex);
// 
// 		// calculate client size
// 
// 		RECT clientSize;
// 		clientSize.top = 0;
// 		clientSize.left = 0;
// 		clientSize.right = currentMode.width;
// 		clientSize.bottom = currentMode.height;
// 
// 		ULONG style = WINDOWED_STYLE;
// 
// 		// Create the rendering window
// 		if (isFullscreen)
// 		{
// 			style  = WS_VISIBLE | WS_POPUP;
// 		} // End if Fullscreen
// 
// 
// 		AdjustWindowRect(&clientSize, style, FALSE);
// 
// 		int32 realWidth = clientSize.right - clientSize.left;
// 		int32 realHeight = clientSize.bottom - clientSize.top;
// 
// 		int32 windowLeft = -10000;//(GetSystemMetrics(SM_CXSCREEN) - realWidth) / 2;
// 		int32 windowTop = -10000;//(GetSystemMetrics(SM_CYSCREEN) - realHeight) / 2;
// 
// 		if (isFullscreen)
// 		{
// 			windowLeft = 0;
// 			windowTop = 0;
// 		}
// 
// 		// create window
// 		hWindow = CreateWindow( className, L"", style, windowLeft, windowTop, 
// 			realWidth, realHeight,	NULL, NULL, hInstance, NULL);
// 
// 		ShowWindow(hWindow, SW_SHOW);
// 		UpdateWindow(hWindow);
// 
// 		// fix ugly ATI driver bugs. Thanks to ariaci (Taken from Irrlight).
// 		MoveWindow(hWindow, windowLeft, windowTop, realWidth, realHeight, TRUE);
// 	
// #if defined(__DAVAENGINE_DIRECTX9__)
// 		RenderManager::Create(Core::RENDERER_DIRECTX9);
// #elif defined(__DAVAENGINE_OPENGL__)
// 		RenderManager::Create(Core::RENDERER_OPENGL);
// #endif
// 		RenderManager::Instance()->Create(hInstance, hWindow);
// 
// 		FrameworkDidLaunched();
// 		KeyedArchive * options = Core::GetOptions();
// 
// 		//fullscreenMode = GetCurrentDisplayMode();
// 		fullscreenMode = GetCurrentDisplayMode();//FindBestMode(fullscreenMode);
// 		if (options)
// 		{
// 			windowedMode.width = options->GetInt32("width");
// 			windowedMode.height = options->GetInt32("height");
// 			windowedMode.bpp = options->GetInt32("bpp");
// 			
// 			// get values from config in case if they are available
// 			fullscreenMode.width = options->GetInt32("fullscreen.width", fullscreenMode.width);
// 			fullscreenMode.height = options->GetInt32("fullscreen.height", fullscreenMode.height);
// 			fullscreenMode.bpp = windowedMode.bpp;
// 
// 			fullscreenMode = FindBestMode(fullscreenMode);
// 
// 			isFullscreen = (0 != options->GetInt32("fullscreen"));	
// 			String title = options->GetString("title", "[set application title using core options property 'title']");
// 			WideString titleW = StringToWString(title);
// 			SetWindowText(hWindow, titleW.c_str());
// 		}
// 
// 		Logger::Info("[PlatformWin32] best display fullscreen mode matched: %d x %d x %d refreshRate: %d", fullscreenMode.width, fullscreenMode.height, fullscreenMode.bpp, fullscreenMode.refreshRate);
// 
// 		currentMode = windowedMode;
// 		if (isFullscreen)
// 		{
// 			currentMode = fullscreenMode;
// 		}
// 
// 		clientSize.top = 0;
// 		clientSize.left = 0;
// 		clientSize.right = currentMode.width;
// 		clientSize.bottom = currentMode.height;
// 
// 		AdjustWindowRect(&clientSize, style, FALSE);
// 
// 		realWidth = clientSize.right - clientSize.left;
// 		realHeight = clientSize.bottom - clientSize.top;
// 
// 		windowLeft = (GetSystemMetrics(SM_CXSCREEN) - realWidth) / 2;
// 		windowTop = (GetSystemMetrics(SM_CYSCREEN) - realHeight) / 2;
// 		MoveWindow(hWindow, windowLeft, windowTop, realWidth, realHeight, TRUE);
// 	
// 
// 		RenderManager::Instance()->ChangeDisplayMode(currentMode, isFullscreen);
// 		RenderManager::Instance()->Init(currentMode.width, currentMode.height);
// 		UIControlSystem::Instance()->SetInputScreenAreaSize(currentMode.width, currentMode.height);
// 		Core::Instance()->SetPhysicalScreenSize(currentMode.width, currentMode.height);
// 
// 		return true;
// 	}


	bool CoreWin32Platform::SetupWindow(HINSTANCE hInstance, HWND hWindow)
	{
		//single instance check
		TCHAR fileName[MAX_PATH];
		GetModuleFileName(NULL, fileName, MAX_PATH);
		fileName[MAX_PATH-1] = 0; //string can be not null-terminated on winXP
		for(int32 i = 0; i < MAX_PATH; ++i)
		{
			if(fileName[i] == L'\\') //symbol \ is not allowed in CreateMutex mutex name
			{
				fileName[i] = ' ';
			}
		}
		hMutex = CreateMutex(NULL, FALSE, fileName);
		if(ERROR_ALREADY_EXISTS == GetLastError())
		{
			return false;
		}

		windowedMode = DisplayMode(800, 600, 16, 0);
		fullscreenMode = DisplayMode(800, 600, 16, 0);
		currentMode = windowedMode;
		isFullscreen = false;

		return true;
	}

	void CoreWin32Platform::Run()
	{
		Core::Instance()->SystemAppStarted();

		MSG msg;
		while(1)
		{
			// process messages
			willQuit = false;
			while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);

				if(msg.message == WM_QUIT)
				{
					ApplicationCore * appCore = Core::Instance()->GetApplicationCore();
					if(appCore && appCore->OnQuit())
					{
						exit(0);
					}
					else
					{
						willQuit = true;
					}
				}
			}
			Sleep(1);
			RenderManager::Instance()->Lock();
			Core::SystemProcessFrame();
			RenderManager::Instance()->Unlock();

			if (willQuit)
			{	
				break;
			}
		}

		Core::Instance()->SystemAppFinished();
		FrameworkWillTerminate();
	}

/*	void CoreWin32Platform::InitOpenGL()
	{
		hDC = GetDC(hWindow);

		PIXELFORMATDESCRIPTOR pfd;
		ZeroMemory( &pfd, sizeof( pfd ) );
		pfd.nSize = sizeof( pfd );
		pfd.nVersion = 1;
		pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
		pfd.iPixelType = PFD_TYPE_RGBA;
		pfd.cColorBits = 24;
		pfd.cDepthBits = 16;
		pfd.iLayerType = PFD_MAIN_PLANE;
		int iFormat = ChoosePixelFormat(hDC, &pfd);
		SetPixelFormat(hDC, iFormat, &pfd);

		hRC = wglCreateContext(hDC);
		Thread::secondaryContext = wglCreateContext(hDC);
		Thread::currentDC = hDC;
		
		wglShareLists(Thread::secondaryContext, hRC);
		wglMakeCurrent(hDC, hRC);

		Thread * t = Thread::Create(Message());
		t->EnableCopyContext();
		t->Start();

		glewInit();
		const GLubyte * extensions = glGetString(GL_EXTENSIONS);
		Logger::Debug("[CoreWin32Platform] gl extensions: %s", (const char*)extensions);
	}*/

	void CoreWin32Platform::ReleaseOpenGL()
	{
		wglMakeCurrent(0, 0);
		wglDeleteContext(hRC);
		ReleaseDC(hWindow, hDC);		
	}
	
// 	void CoreWin32Platform::ToggleFullscreen()
// 	{
// 		
// 		
// 	}

/*	void CoreWin32Platform::SwitchToFullScreen() 
	{
// 		DEVMODE dmScreenSettings;
// 
// 		memset(&dmScreenSettings, 0, sizeof(dmScreenSettings));
// 		dmScreenSettings.dmSize = sizeof(dmScreenSettings);
// 		dmScreenSettings.dmPelsWidth = screenWidth;
// 		dmScreenSettings.dmPelsHeight = screenHeight;
// 		dmScreenSettings.dmBitsPerPel = 32;
// 		dmScreenSettings.dmFields = DM_BITSPERPEL|DM_PELSWIDTH|DM_PELSHEIGHT;
// 
// 		ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN);

		LONG dwExStyle = GetWindowLong(hWindow, GWL_EXSTYLE);
		LONG dwStyle = GetWindowLong(hWindow, GWL_STYLE);

		//dwExStyle &= ~(WS_EX_DLGMODALFRAME | WS_EX_CLIENTEDGE | WS_EX_STATICEDGE);
		//dwStyle &= ~(WS_CAPTION | WS_THICKFRAME | WS_MINIMIZE | WS_MAXIMIZE | WS_SYSMENU);

		SetWindowLong(hWindow, GWL_EXSTYLE, dwExStyle);
		SetWindowLong(hWindow, GWL_STYLE, dwStyle);

		SetWindowPos(hWindow, 0, 0, 0, screenWidth, screenHeight, SWP_NOZORDER | SWP_FRAMECHANGED);
		isInFullscreenNow = true;
	}

	void CoreWin32Platform::CloseFullScreen() 
	{
//		ChangeDisplaySettings(NULL, 0);
		ShowCursor(TRUE);

		LONG dwExStyle = GetWindowLong(hWindow, GWL_EXSTYLE);
		LONG dwStyle = GetWindowLong(hWindow, GWL_STYLE);

		//	dwExStyle |= (WS_EX_DLGMODALFRAME | WS_EX_CLIENTEDGE | WS_EX_STATICEDGE);
		//	dwStyle |= (WS_CAPTION | WS_THICKFRAME | WS_MINIMIZE | WS_MAXIMIZE | WS_SYSMENU);
		

		dwExStyle |= WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
		dwStyle |= WS_OVERLAPPEDWINDOW;

		SetWindowLong(hWindow, GWL_EXSTYLE, dwExStyle);
		SetWindowLong(hWindow, GWL_STYLE, dwStyle);

		SetWindowPos(hWindow, HWND_TOP, 0, 0, screenWidth, screenHeight, SWP_FRAMECHANGED);
		isInFullscreenNow = false;
	}
*/
	RECT CoreWin32Platform::GetWindowedRectForDisplayMode(DisplayMode & dm)
	{
		RECT clientSize;
		clientSize.top = 0;
		clientSize.left = 0;
		clientSize.right = dm.width;
		clientSize.bottom = dm.height;

		AdjustWindowRect(&clientSize, GetWindowLong(hWindow, GWL_STYLE), FALSE);

		return clientSize;
	}

	void CoreWin32Platform::ToggleFullscreen()
	{
		// Setup styles based on windowed / fullscreen mode
		isFullscreen = !isFullscreen;

		if ( isFullscreen )
		{
			currentMode = fullscreenMode;
			GetWindowRect(hWindow, &windowPositionBeforeFullscreen);

			SetMenu( hWindow, NULL );
			SetWindowLong( hWindow, GWL_STYLE, FULLSCREEN_STYLE );
			SetWindowPos( hWindow, NULL, 0, 0, currentMode.width, currentMode.height, SWP_NOZORDER );
		} 
		else
		{
			SetWindowLong( hWindow, GWL_STYLE, WINDOWED_STYLE );

			currentMode = windowedMode;
			RECT windowedRect = GetWindowedRectForDisplayMode(currentMode);
	
			SetWindowPos( hWindow, HWND_NOTOPMOST, windowPositionBeforeFullscreen.left, windowPositionBeforeFullscreen.top, windowedRect.right - windowedRect.left, windowedRect.bottom - windowedRect.top, SWP_NOACTIVATE | SWP_SHOWWINDOW );
		}
		
		Logger::Debug("[RenderManagerDX9] toggle mode: %d x %d isFullscreen: %d", currentMode.width, currentMode.height, isFullscreen);

		RenderManager::Instance()->ChangeDisplayMode(currentMode, isFullscreen);
		RenderManager::Instance()->Init(currentMode.width, currentMode.height);
		UIControlSystem::Instance()->SetInputScreenAreaSize(currentMode.width, currentMode.height);
		Core::Instance()->SetPhysicalScreenSize(currentMode.width, currentMode.height);
	}

	Core::eScreenMode CoreWin32Platform::GetScreenMode()
	{
		if (isFullscreen)return Core::MODE_FULLSCREEN;
		else return Core::MODE_WINDOWED;
	}

	void CoreWin32Platform::SwitchScreenToMode(eScreenMode screenMode)
	{
		if (GetScreenMode() != screenMode) // check if we try to switch mode
		{
			if (screenMode == Core::MODE_FULLSCREEN)
			{
				ToggleFullscreen();
			}else if (screenMode == Core::MODE_WINDOWED)
			{
				ToggleFullscreen();
			}
		}else
		{
		}
	}

	void CoreWin32Platform::GetAvailableDisplayModes(List<DisplayMode> & availableDisplayModes)
	{
		availableDisplayModes.clear();

		DWORD iModeNum = 0;
		DEVMODE	dmi;
		ZeroMemory (&dmi, sizeof(dmi)) ;
		dmi.dmSize = sizeof(dmi) ;

		while(EnumDisplaySettings(NULL, iModeNum++, &dmi))
		{
			DisplayMode mode;
			mode.width = dmi.dmPelsWidth;
			mode.height = dmi.dmPelsHeight;
			mode.bpp = dmi.dmBitsPerPel;
			mode.refreshRate = dmi.dmDisplayFrequency;
			ZeroMemory (&dmi, sizeof(dmi)) ;
			availableDisplayModes.push_back(mode);

			Logger::Debug(L"[RenderManagerDX9::GetAvailableDisplayModes] mode found: %d x %d x %d",
				mode.width,
				mode.height,
				mode.bpp);
		}
	}

	DisplayMode CoreWin32Platform::GetCurrentDisplayMode()
	{
		DWORD iModeNum = 0;
		DEVMODE	dmi;
		ZeroMemory (&dmi, sizeof(dmi)) ;
		dmi.dmSize = sizeof(dmi);

		DisplayMode mode;
		if(EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &dmi))
		{
			mode.width = dmi.dmPelsWidth;
			mode.height = dmi.dmPelsHeight;
			mode.bpp = dmi.dmBitsPerPel;
			mode.refreshRate = dmi.dmDisplayFrequency;
			ZeroMemory (&dmi, sizeof(dmi)) ;
		}

		return mode;
	}


	void CoreWin32Platform::Quit()
	{
		PostQuitMessage(0);
	}

	void CoreWin32Platform::SetIcon(int32 iconId)
	{
		HINSTANCE hInst= GetModuleHandle(0);
		HICON smallIcon = static_cast<HICON>(LoadImage(hInst,
			MAKEINTRESOURCE(iconId),
			IMAGE_ICON,
			0,
			0,
			LR_DEFAULTSIZE));
		SendMessage(hWindow, WM_SETICON, ICON_SMALL, (LPARAM)smallIcon);
		SendMessage(hWindow, WM_SETICON, ICON_BIG, (LPARAM)smallIcon);
	}

	static Vector<DAVA::UIEvent> activeTouches;
	int32 MoveTouchsToVector(UINT message, WPARAM wParam, LPARAM lParam, Vector<UIEvent> *outTouches)
	{
		
		int button = 0;
		//	case WM_LBUTTONDOWN:
		//	case WM_RBUTTONDOWN:
		//	case WM_MBUTTONDOWN:
		//	case WM_XBUTTONDOWN:
		//		
		//	case WM_LBUTTONUP:
		//	case WM_RBUTTONUP:
		//	case WM_MBUTTONUP:
		//	case WM_XBUTTONUP:
		//		
		//	case WM_MOUSEMOVE:
		if(message == WM_LBUTTONDOWN || message == WM_LBUTTONUP || message == WM_MOUSEMOVE)
		{
			button = 1;
		}
		else if(message == WM_RBUTTONDOWN || message == WM_RBUTTONUP)
		{
			button = 2;
		}
		else if(message == WM_MBUTTONDOWN || message == WM_MBUTTONUP)
		{
			button = 3;
		}
// 		else if(message == WM_XBUTTONDOWN || message == WM_XBUTTONUP)
// 		{
// 			if(HIWORD(wParam) == XBUTTON1)
// 			{
// 				button = 4;
// 			}
// 			else 
// 			{
// 				button = 5;
// 			}
// 
// 		}

		//	NSLog(@"Event button %d", button);

		int phase = UIEvent::PHASE_MOVE;
		if(message == WM_LBUTTONDOWN || message == WM_RBUTTONDOWN || message == WM_MBUTTONDOWN /*|| message == WM_XBUTTONDOWN*/)
		{
			phase = UIEvent::PHASE_BEGAN;
			//		NSLog(@"Event phase PHASE_BEGAN");
		}
		else if(message == WM_LBUTTONUP || message == WM_RBUTTONUP || message == WM_MBUTTONUP /*|| message == WM_XBUTTONUP*/)
		{
			phase = UIEvent::PHASE_ENDED;
			//		NSLog(@"Event phase PHASE_ENDED");
		}
		else if(LOWORD(wParam)&MK_LBUTTON || LOWORD(wParam)&MK_RBUTTON || LOWORD(wParam)&MK_MBUTTON /*|| LOWORD(wParam)&MK_XBUTTON1 || LOWORD(wParam)&MK_XBUTTON2*/)
		{
			phase = UIEvent::PHASE_DRAG;
		}

		if(phase == UIEvent::PHASE_DRAG)
		{
			for(Vector<DAVA::UIEvent>::iterator it = activeTouches.begin(); it != activeTouches.end(); it++)
			{
				Vector2 p((float32)GET_X_LPARAM(lParam), (float32)GET_Y_LPARAM(lParam));

				it->physPoint.x = p.x;
				it->physPoint.y = p.y;
				it->phase = phase;
			}
		}

		bool isFind = false;
		for(Vector<DAVA::UIEvent>::iterator it = activeTouches.begin(); it != activeTouches.end(); it++)
		{
			if(it->tid == button)
			{
				isFind = true;

				Vector2 p((float32)GET_X_LPARAM(lParam), (float32)GET_Y_LPARAM(lParam));

				it->physPoint.x = p.x;
				it->physPoint.y = p.y;
				it->phase = phase;
				//				it->timestamp = curEvent.timestamp;
				//				it->tapCount = curEvent.clickCount;
				it->phase = phase;

				break;
			}
		}

		if(!isFind)
		{
			UIEvent newTouch;
			newTouch.tid = button;
			Vector2 p((float32)GET_X_LPARAM(lParam), (float32)GET_Y_LPARAM(lParam));
			newTouch.physPoint.x = p.x;
			newTouch.physPoint.y = p.y;
			//			newTouch.timestamp = curEvent.timestamp;
			//			newTouch.tapCount = curEvent.clickCount;
			newTouch.phase = phase;
			activeTouches.push_back(newTouch);
		}

		for(Vector<DAVA::UIEvent>::iterator it = activeTouches.begin(); it != activeTouches.end(); it++)
		{
			outTouches->push_back(*it);
		}

		if(phase == UIEvent::PHASE_ENDED || phase == UIEvent::PHASE_MOVE)
		{
			for(Vector<DAVA::UIEvent>::iterator it = activeTouches.begin(); it != activeTouches.end(); it++)
			{
				if(it->tid == button)
				{
					activeTouches.erase(it);
					break;
				}
			}
		}
		return phase;
	}


	//static bool mouseShow = false;
	static bool mouseCursorShown = true;


	bool CoreWin32Platform::WinEvent(MSG *message, long *result)
	{

#ifndef WM_MOUSEWHEEL
#define WM_MOUSEWHEEL 0x020A
#endif
#ifndef WHEEL_DELTA                     
#define WHEEL_DELTA 120
#endif

//		Logger::Debug("Event: %d(%0x)", message->message, message->message);

		switch (message->message) 
		{
		case WM_KEYUP:
			{
				InputSystem::Instance()->GetKeyboard()->OnSystemKeyUnpressed((int32)message->wParam);
				*result = 0;
				return true;
			};

		case WM_KEYDOWN:
			{
				BYTE allKeys[256];
				GetKeyboardState(allKeys);

				Vector<DAVA::UIEvent> touches;
				Vector<DAVA::UIEvent> emptyTouches;

				for(Vector<DAVA::UIEvent>::iterator it = activeTouches.begin(); it != activeTouches.end(); it++)
				{
					touches.push_back(*it);
				}

				DAVA::UIEvent ev;
				ev.keyChar = 0;
				ev.phase = DAVA::UIEvent::PHASE_KEYCHAR;
				ev.tapCount = 1;
				ev.tid = InputSystem::Instance()->GetKeyboard()->GetDavaKeyForSystemKey((int32)message->wParam);

				touches.push_back(ev);

				UIControlSystem::Instance()->OnInput(0, emptyTouches, touches);
				touches.pop_back();
				UIControlSystem::Instance()->OnInput(0, emptyTouches, touches);

				InputSystem::Instance()->GetKeyboard()->OnSystemKeyPressed((int32)message->wParam);

				*result = 0;
				return true;
			};

		case WM_CHAR:
			{
				if(message->wParam > 27) //TODO: remove this elegant check
				{
					Vector<DAVA::UIEvent> touches;
					Vector<DAVA::UIEvent> emptyTouches;

					for(Vector<DAVA::UIEvent>::iterator it = activeTouches.begin(); it != activeTouches.end(); it++)
					{
						touches.push_back(*it);
					}

					DAVA::UIEvent ev;
					ev.keyChar = (char16)message->wParam;
					ev.phase = DAVA::UIEvent::PHASE_KEYCHAR;
					ev.tapCount = 1;
					ev.tid = 0;

					touches.push_back(ev);

					UIControlSystem::Instance()->OnInput(0, emptyTouches, touches);
					touches.pop_back();
					UIControlSystem::Instance()->OnInput(0, emptyTouches, touches);
				}
				*result = 0;
				return true;
			}
			break;

		case WM_LBUTTONDOWN:
		case WM_RBUTTONDOWN:
		case WM_MBUTTONDOWN:
			//		case WM_XBUTTONDOWN:

		case WM_LBUTTONUP:
		case WM_RBUTTONUP:
		case WM_MBUTTONUP:
			//		case WM_XBUTTONUP:

		case WM_MOUSEMOVE:
			{
				//Logger::Debug("ms: %d %d", GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));

				Vector<DAVA::UIEvent> touches;
				Vector<DAVA::UIEvent> emptyTouches;

				int32 touchPhase = MoveTouchsToVector(message->message, message->wParam, message->lParam, &touches);

				UIControlSystem::Instance()->OnInput(touchPhase, emptyTouches, touches);

				touches.clear();

// 				*result = 0;
// 				return true;
				break;
			}
		}

		return false;
	}
	
}
#endif // #if defined(__DAVAENGINE_WIN32__)