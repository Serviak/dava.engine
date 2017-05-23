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
    DigitalElementState GetDigitalElementState(eInputElements elementId) const override;
    AnalogElementState GetAnalogElementState(eInputElements elementId) const override;

    /** Translate keyboard key into wide string, using curring keyboard layout. */
    WideString TranslateElementToWideString(eInputElements elementId) const;

    /** Convert keyboard input element to native platform scancode */
    uint32 GetElementNativeScancode(eInputElements elementId) const;

private:
    Keyboard(uint32 id);
    ~Keyboard();
    Keyboard(const Keyboard&) = delete;
    Keyboard& operator=(const Keyboard&) = delete;

    void OnEndFrame();
    void OnWindowFocusChanged(DAVA::Window* window, bool focused);

    bool HandleMainDispatcherEvent(const Private::MainDispatcherEvent& e);
    void CreateAndSendKeyInputEvent(eInputElements elementId, DigitalElementState state, Window* window, int64 timestamp);
    void CreateAndSendCharInputEvent(char32_t charCode, bool charRepeated, Window* window, int64 timestamp);

private:
    InputSystem* inputSystem = nullptr;
    Private::KeyboardImpl* impl = nullptr;

    // State of each physical key
    Array<DigitalElementState, INPUT_ELEMENTS_KB_COUNT> keys;
};
} // namespace DAVA
