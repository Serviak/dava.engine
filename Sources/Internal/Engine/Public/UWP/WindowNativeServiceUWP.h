#pragma once

#if defined(__DAVAENGINE_COREV2__)

#include "Base/BaseTypes.h"

#if defined(__DAVAENGINE_WIN_UAP__)

#include "Engine/Private/EnginePrivateFwd.h"

namespace DAVA
{
class WindowNativeService final
{
public:
    void AddXamlControl(Windows::UI::Xaml::UIElement ^ xamlControl);
    void RemoveXamlControl(Windows::UI::Xaml::UIElement ^ xamlControl);
    void PositionXamlControl(Windows::UI::Xaml::UIElement ^ xamlControl, float32 x, float32 y);
    void UnfocusXamlControl();

private:
    WindowNativeService(Private::WindowNativeBridge ^ cxxBridge);

private:
    Private::WindowNativeBridge ^ bridge = nullptr;

    // Friends
    friend Private::WindowBackend;
};

} // namespace DAVA

#endif // __DAVAENGINE_WIN_UAP__
#endif // __DAVAENGINE_COREV2__
