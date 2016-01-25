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


#ifndef __DAVAENGINE_CORE_PLATFORM_WIN32_H__
#define __DAVAENGINE_CORE_PLATFORM_WIN32_H__

#include "Base/Platform.h"
#if defined(__DAVAENGINE_WIN32__)

#include "CoreWin32PlatformBase.h"
#include "UI/UIEvent.h"

namespace DAVA {

class CoreWin32Platform : public CoreWin32PlatformBase
{
public:
	eScreenMode GetScreenMode() override;
    bool SetScreenMode(eScreenMode screenMode) override;
    void GetAvailableDisplayModes(List<DisplayMode>& availableModes) override;

    DisplayMode GetCurrentDisplayMode() override;

    bool CreateWin32Window(HINSTANCE hInstance); //true if window created, if false, need to quit the app
    void Run();

    void SetIcon(int32 iconId) override;

    void SetWindowMinimumSize(float32 width, float32 height) override;
    Vector2 GetWindowMinimumSize() const override;

private:
    DisplayMode currentMode;
    DisplayMode fullscreenMode;
    DisplayMode windowedMode;
    bool isFullscreen;
    RECT windowPositionBeforeFullscreen;

    static const uint32 WINDOWED_STYLE = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
    static const uint32 FULLSCREEN_STYLE = WS_VISIBLE | WS_POPUP;

    void OnMouseMove(float32 x, float32 y);
    void OnMouseWheel(float32 wheelDelta, float32 x, float32 y);
    void OnMouseButtonChange(UIEvent::Phase phase, UIEvent::MouseButton button, float32 x, float32 y);
    void OnTouchEvent(UIEvent::Phase phase, UIEvent::Device deviceId, uint32 fingerId, float32 x, float32 y, float presure);
    void OnGetMinMaxInfo(MINMAXINFO* minmaxInfo);
    static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

    RECT GetWindowedRectForDisplayMode(DisplayMode& dm);
    void LoadWindowMinimumSizeSettings();

    bool willQuit;

    Bitset<static_cast<size_t>(UIEvent::MouseButton::NUM_BUTTONS)> mouseButtonState;
    Vector<TOUCHINPUT> inputTouchBuffer;

    float32 minWindowWidth = 0.0f;
    float32 minWindowHeight = 0.0f;
};

} // end namespace DAVA
#endif // #if defined(__DAVAENGINE_WIN32__)
#endif // __DAVAENGINE_CORE_PLATFORM_WIN32_H__
