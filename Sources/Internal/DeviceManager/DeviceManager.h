#pragma once

#include "Base/BaseTypes.h"

#if defined(__DAVAENGINE_COREV2__)

#include "DeviceManager/DeviceManagerTypes.h"
#include "Input/TouchDevice.h"
#include "Engine/Private/EnginePrivateFwd.h"
#include "Functional/Signal.h"

/**
    \defgroup device_manager Device Manager
*/

namespace DAVA
{
class GamepadDevice;
class InputDevice;
class KeyboardInputDevice;
class MouseDevice;
namespace Private
{
struct DeviceManagerImpl;
struct MainDispatcherEvent;
}

/**
    \ingroup device_manager

    Class which keeps current device configuration, listens for device addition, removal or devices' properties changes.
    Application can subscribe to appropriate signals to receive notification about configuration changes.

    \todo For now `DeviceManager` observes only display devices, further add other devices (input, storage, maybe network).
*/
class DeviceManager final
{
private:
    DeviceManager(Private::EngineBackend* engineBackend);
    ~DeviceManager();

public:
    // Display methods

    /** Get primary display as reported by system */
    const DisplayInfo& GetPrimaryDisplay() const;

    /** Get displays which are available now */
    const Vector<DisplayInfo>& GetDisplays() const;

    /** Get total display count */
    size_t GetDisplayCount() const;

    // Input methods

    InputDevice* GetInputDevice(uint32 id);
    GamepadDevice* GetGamepad();
    KeyboardInputDevice* GetKeyboard();
    MouseDevice* GetMouse();
    TouchDevice* GetTouch();

    // Signals

    Signal<> displayConfigChanged; //<! Emited when display has been added/removed or properties of any display have changed

private:
    void UpdateDisplayConfig();

    bool HandleEvent(const Private::MainDispatcherEvent& e);
    void HandleDisplayConfigChanged(const Private::MainDispatcherEvent& e);
    void HandleGamepadMotion(const Private::MainDispatcherEvent& e);
    void HandleGamepadButton(const Private::MainDispatcherEvent& e);
    void HandleGamepadAdded(const Private::MainDispatcherEvent& e);
    void HandleGamepadRemoved(const Private::MainDispatcherEvent& e);

    void Update(float32 frameDelta);
    void OnEngineInited();

    Vector<DisplayInfo> displays;

    KeyboardInputDevice* keyboard = nullptr;
    MouseDevice* mouse = nullptr;
    GamepadDevice* gamepad = nullptr;
    TouchDevice* touch = nullptr;
    Vector<InputDevice*> inputDevices;

    std::unique_ptr<Private::DeviceManagerImpl> impl;

    friend class Private::EngineBackend;
    friend struct Private::DeviceManagerImpl;
};

inline const DisplayInfo& DeviceManager::GetPrimaryDisplay() const
{
    // DeviceManagerImpl always places primary display as first element
    return displays[0];
}

inline const Vector<DisplayInfo>& DeviceManager::GetDisplays() const
{
    return displays;
}

inline size_t DeviceManager::GetDisplayCount() const
{
    return displays.size();
}

} // namespace DAVA

#endif // __DAVAENGINE_COREV2__
