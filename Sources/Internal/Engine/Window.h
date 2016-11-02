#pragma once

#if defined(__DAVAENGINE_COREV2__)

#include "Base/BaseTypes.h"
#include "Functional/Signal.h"
#include "Math/Vector.h"

#include "UI/UIEvent.h"

#include "Engine/Private/EnginePrivateFwd.h"
#include "Engine/Private/EngineBackend.h"

namespace rhi
{
struct InitParam;
}

namespace DAVA
{
class InputSystem;
class UIControlSystem;
class VirtualCoordinatesSystem;

/** Window class. */
class Window final
{
    friend class Private::EngineBackend;
    friend class Private::PlatformCore;

private:
    Window(Private::EngineBackend* engineBackend, bool primary);
    ~Window();

    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;

public:
    bool IsPrimary() const;
    bool IsVisible() const;
    bool HasFocus() const;

    /** 
        Returns dots-per-inch for a monitor, where that window is placed. 
        
        \remark Use `Window::dpiChanged` signal to know, when window was placed on other monitor with other dpi.
    */
    float32 GetDPI() const;

    /** 
        Returns size of the window's client area. 
        Window size in screen coordinates may differ from the size in pixels,
        if the windows was created on system with high-dpi support (e.g. OSX or Windows 10).

        \remark Use `GetSurfaceSize()` to get rendering surface size in pixels.
        \remark Use `Window::sizeChanged` signal to know, when window size was changed.
    */
    Size2f GetSize() const;

    /** 
        Sets the size of a window's client area. 
        Window size in screen coordinates may differ from the size in pixels,
        if the windows was created on system with high-dpi support (e.g. OSX or Windows 10).
        On some platforms (iOS, Android or Win10 Phone) there is no real window system and
        as a consequence window size can't be changes, so this function will have no effect.

        \remark Use `Window::sizeChanged` signal to know, when window size was changed.
    */
    void SetSize(Size2f size);

    /**
         Returns size of the window's rendering surface in pixels.
         Surface size is in raw pixels.

         \remark Use `Window::sizeChanged` signal to know, when window surface size was changed.
         \remark Use `SetSurfaceScale` to tune surface size.
    */
    Size2f GetSurfaceSize() const;

    /** 
        Returns window rendering surface scale. 
        By default it is 1.0f unit user changes it with `SetSurfaceScale()` method.
    */
    float32 GetSurfaceScale() const;

    /** 
        Sets window rendering surface scale.
        Return `true` if scaling was successfully set. Scale value has to be from `0.0f` to `1.0f`.
        
        \remark This should be used by user to tune rendering surface size for performance reason.
    */
    bool SetSurfaceScale(float32 scale);

    void Close();
    void SetTitle(const String& title);

    Engine* GetEngine() const;
    void* GetNativeHandle() const;
    WindowNativeService* GetNativeService() const;

    void RunAsyncOnUIThread(const Function<void()>& task);

public:
    // Signals
    Signal<Window*, bool> visibilityChanged;
    Signal<Window*, bool> focusChanged;
    Signal<Window*, float32> dpiChanged;
    Signal<Window*, Size2f, Size2f> sizeChanged; //<! First Size2f is window size, second Size2f is window surface size
    //Signal<Window*> beginUpdate;
    //Signal<Window*> beginDraw;
    Signal<Window*, float32> update;
    //Signal<Window*> endDraw;
    //Signal<Window*> endUpdate;

private:
    /// Get pointer to WindowBackend which may be used by PlatformCore
    Private::WindowBackend* GetBackend() const;

    /// Initialize platform specific render params, e.g. acquire/release context functions for Qt platform
    void InitCustomRenderParams(rhi::InitParam& params);
    void Update(float32 frameDelta);
    void Draw();

    /// Process main dispatcher events targeting this window
    void EventHandler(const Private::MainDispatcherEvent& e);
    /// Do some window specific tasks after all dispatcher events have been processed on current frame,
    /// e.g. initiate processing tasks on window UI thread
    void FinishEventHandlingOnCurrentFrame();

    void HandleWindowCreated(const Private::MainDispatcherEvent& e);
    void HandleWindowDestroyed(const Private::MainDispatcherEvent& e);
    void HandleSizeChanged(const Private::MainDispatcherEvent& e);
    void HandleDpiChanged(const Private::MainDispatcherEvent& e);
    void HandleFocusChanged(const Private::MainDispatcherEvent& e);
    void HandleVisibilityChanged(const Private::MainDispatcherEvent& e);
    void HandleMouseClick(const Private::MainDispatcherEvent& e);
    void HandleMouseWheel(const Private::MainDispatcherEvent& e);
    void HandleMouseMove(const Private::MainDispatcherEvent& e);
    void HandleTouchClick(const Private::MainDispatcherEvent& e);
    void HandleTouchMove(const Private::MainDispatcherEvent& e);
    void HandleKeyPress(const Private::MainDispatcherEvent& e);
    void HandleKeyChar(const Private::MainDispatcherEvent& e);

    void MergeSizeChangedEvents(const Private::MainDispatcherEvent& e);
    void UpdateVirtualCoordinatesSystem();
    void ClearMouseButtons();

private:
    Private::EngineBackend* engineBackend = nullptr;
    Private::MainDispatcher* mainDispatcher = nullptr;
    std::unique_ptr<Private::WindowBackend> windowBackend;

    InputSystem* inputSystem = nullptr;
    UIControlSystem* uiControlSystem = nullptr;

    bool isPrimary = false;
    bool isVisible = false;
    bool hasFocus = false;
    bool sizeEventsMerged = false; // Flag indicating that all size events are merged on current frame

    float32 dpi = 0.0f; //!< Window DPI
    float32 width = 0.0f; //!< Window client area width.
    float32 height = 0.0f; //!< Window client area height.
    float32 surfaceWidth = 0.0f; //!< Window rendering surface width.
    float32 surfaceHeight = 0.0f; //!< Window rendering surface height.
    float32 surfaceScale = 1.0f; //!< Window rendering surface scale.

    Bitset<static_cast<size_t>(UIEvent::MouseButton::NUM_BUTTONS)> mouseButtonState;
};

inline bool Window::IsPrimary() const
{
    return isPrimary;
}

inline bool Window::IsVisible() const
{
    return isVisible;
}

inline bool Window::HasFocus() const
{
    return hasFocus;
}

inline float32 Window::GetDPI() const
{
    return dpi;
}

inline Size2f Window::GetSize() const
{
    return { width, height };
}

inline Size2f Window::GetSurfaceSize() const
{
    return { surfaceWidth, surfaceHeight };
}

inline float32 Window::GetSurfaceScale() const
{
    return surfaceScale;
}

inline Private::WindowBackend* Window::GetBackend() const
{
    return windowBackend.get();
}

} // namespace DAVA

#endif // __DAVAENGINE_COREV2__
