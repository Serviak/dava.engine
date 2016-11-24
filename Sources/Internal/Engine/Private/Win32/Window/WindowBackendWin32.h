#pragma once

#include "Base/BaseTypes.h"

#if defined(__DAVAENGINE_COREV2__)
#if defined(__DAVAENGINE_QT__)
// TODO: plarform defines
#elif defined(__DAVAENGINE_WIN32__)

#include <bitset>

#include "Base/Platform.h"
#include "Engine/EngineTypes.h"
#include "Engine/Private/EnginePrivateFwd.h"
#include "Engine/Private/Dispatcher/UIDispatcher.h"
#include "Engine/EngineTypes.h"

namespace rhi
{
struct InitParam;
}

namespace DAVA
{
namespace Private
{
class WindowBackend final
{
public:
    WindowBackend(EngineBackend* engineBackend, Window* window);
    ~WindowBackend();

    WindowBackend(const WindowBackend&) = delete;
    WindowBackend& operator=(const WindowBackend&) = delete;

    bool Create(float32 width, float32 height);
    void Resize(float32 width, float32 height);
    void Close(bool appIsTerminating);
    void SetTitle(const String& title);

    eFullscreen GetFullscreen() const;
    void SetFullscreen(eFullscreen newMode);

    void RunAsyncOnUIThread(const Function<void()>& task);
    void RunAndWaitOnUIThread(const Function<void()>& task);

    void* GetHandle() const;
    HWND GetHWND() const;

    bool IsWindowReadyForRender() const;
    void InitCustomRenderParams(rhi::InitParam& params);

    void TriggerPlatformEvents();
    void ProcessPlatformEvents();

    void SetCursorCapture(eCursorCapture mode);
    void SetCursorVisibility(bool visible);

private:
    // Shortcut for eMouseButtons::COUNT
    static const size_t MOUSE_BUTTON_COUNT = static_cast<size_t>(eMouseButtons::COUNT);

    void SetCursorInCenter();
    void DoResizeWindow(float32 width, float32 height);
    void DoCloseWindow();
    void DoSetTitle(const char8* title);
    void DoSetFullscreen(eFullscreen newMode);

    void SetFullscreenMode();
    void SetWindowedMode();
    void DoSetCursorCapture(eCursorCapture mode);
    void DoSetCursorVisibility(bool visible);
    void UpdateClipCursor();
    void HandleWindowFocusChanging(bool focusState);

    void AdjustWindowSize(int32* w, int32* h);
    void HandleSizeChanged(int32 w, int32 h);

    void UIEventHandler(const UIDispatcherEvent& e);

    LRESULT OnSize(int32 resizingType, int32 width, int32 height);
    LRESULT OnEnterSizeMove();
    LRESULT OnExitSizeMove();
    LRESULT OnGetMinMaxInfo(MINMAXINFO* minMaxInfo);
    LRESULT OnDpiChanged(RECT* suggestedRect);
    LRESULT OnActivate(WPARAM wparam);
    LRESULT OnMouseMoveEvent(int32 x, int32 y);
    LRESULT OnMouseMoveRelativeEvent(int x, int y);
    LRESULT OnMouseWheelEvent(int32 deltaX, int32 deltaY, int32 x, int32 y);
    LRESULT OnMouseClickEvent(UINT message, uint16 xbutton, int32 x, int32 y);
    LRESULT OnCaptureChanged();
    LRESULT OnTouch(uint32 ntouch, HTOUCHINPUT htouch);
    LRESULT OnPointerClick(uint32 pointerId, int32 x, int32 y);
    LRESULT OnPointerUpdate(uint32 pointerId, int32 x, int32 y);
    LRESULT OnKeyEvent(uint32 key, uint32 scanCode, bool isPressed, bool isExtended, bool isRepeated);
    LRESULT OnCharEvent(uint32 key, bool isRepeated);
    LRESULT OnCreate();
    LRESULT OnSetCursor(LPARAM lparam);
    bool OnClose();
    LRESULT OnDestroy();
    LRESULT WindowProc(UINT message, WPARAM wparam, LPARAM lparam, bool& isHandled);
    static LRESULT CALLBACK WndProcStart(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);
    static bool RegisterWindowClass();
    static eModifierKeys GetModifierKeys();
    static eInputDevices GetInputEventSourceLegacy(LPARAM messageExtraInfo);
    static eMouseButtons GetMouseButtonLegacy(uint32 curState, uint32 newState, bool* isPressed);
    static eMouseButtons GetMouseButton(POINTER_BUTTON_CHANGE_TYPE buttonChangeType, bool* isPressed);

    float32 GetDpi() const;

private:
    eCursorCapture captureMode = eCursorCapture::OFF;
    bool mouseVisible = true;
    HCURSOR defaultCursor = nullptr;
    POINT lastCursorPosition;

    EngineBackend* engineBackend = nullptr;
    Window* window = nullptr; // Window frontend reference
    MainDispatcher* mainDispatcher = nullptr; // Dispatcher that dispatches events to DAVA main thread
    UIDispatcher uiDispatcher; // Dispatcher that dispatches events to window UI thread

    HWND hwnd = nullptr;

    bool isMinimized = false;
    bool hasFocus = false;

    bool isEnteredSizingModalLoop = false;
    bool closeRequestByApp = false;
    bool isFullscreen = false;
    int32 lastWidth = 0; // Track current window size to not post excessive WINDOW_SIZE_CHANGED events
    int32 lastHeight = 0;
    int32 lastMouseMoveX = -1; // Remember last mouse move position to detect
    int32 lastMouseMoveY = -1; // spurious mouse move events
    uint32 mouseButtonsState = 0; // Mouse buttons state for legacy mouse events (not new pointer input events)

    const float32 defaultDpi = 96.0f;
    float32 dpi = defaultDpi;
    Vector<TOUCHINPUT> touchInput;
    WINDOWPLACEMENT windowPlacement;

    static bool windowClassRegistered;
    static const wchar_t windowClassName[];
    static const UINT WM_TRIGGER_EVENTS = WM_USER + 39;
    static const DWORD windowedStyle = WS_OVERLAPPEDWINDOW;
    static const DWORD fullscreenStyle = WS_POPUP;
    static const DWORD windowExStyle = 0;
};

inline void* WindowBackend::GetHandle() const
{
    return static_cast<void*>(hwnd);
}

inline HWND WindowBackend::GetHWND() const
{
    return hwnd;
}

inline void WindowBackend::InitCustomRenderParams(rhi::InitParam& /*params*/)
{
    // No custom render params
}

inline eFullscreen WindowBackend::GetFullscreen() const
{
    return isFullscreen ? eFullscreen::On : eFullscreen::Off;
}

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_WIN32__
#endif // __DAVAENGINE_COREV2__
