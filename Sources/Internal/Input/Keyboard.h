#pragma once

#include "Input/InputDevice.h"

namespace DAVA
{
class Window;
class InputSystem;

namespace Private
{
class KeyboardImpl;
struct MainDispatcherEvent;
}

/**
    \ingroup input
    Represents keyboard input device.
*/
class Keyboard final : public InputDevice
{
    friend class DeviceManager; // For creation

public:
    // InputDevice overrides
    bool IsElementSupported(eInputElements elementId) const override;
    eDigitalElementStates GetDigitalElementState(eInputElements elementId) const override;
    AnalogElementState GetAnalogElementState(eInputElements elementId) const override;

    /**
        Translate keyboard key into wide string, using curring keyboard layout.
    */
    WideString TranslateElementToWideString(eInputElements elementId) const;

private:
    Keyboard(uint32 id);
    ~Keyboard();
    Keyboard(const Keyboard&) = delete;
    Keyboard& operator=(const Keyboard&) = delete;

    void OnEndFrame();
    void OnWindowFocusChanged(DAVA::Window* window, bool focused);

    bool HandleMainDispatcherEvent(const Private::MainDispatcherEvent& e);
    void CreateAndSendInputEvent(eInputElements elementId, eDigitalElementStates element, Window* window, int64 timestamp) const;

private:
    InputSystem* inputSystem = nullptr;
    std::unique_ptr<Private::KeyboardImpl> impl = nullptr;

    // State of each physical key
    Array<eDigitalElementStates, INPUT_ELEMENTS_KB_COUNT> keys;
};
} // namespace DAVA
