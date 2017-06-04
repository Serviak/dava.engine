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
    /**
        Return state of a specified `key`.

        \note It's essentialy a shorter and more readable placeholder for `GetDigitalElementState(key)`.
        \pre `key` should be a keyboard element.
    */
    DigitalElementState GetKeyState(eInputElements key) const;

    /**
        Translate a keyboard key into a wide string, using curring keyboard layout.

        \pre `elementId` should be a keyboard element.
    */
    String TranslateElementToUTF8String(eInputElements elementId) const;

    /**
        Convert keyboard input element to native platform scancode.

        \pre `elementId` should be a keyboard element.
    */
    uint32 GetKeyNativeScancode(eInputElements elementId) const;

    // InputDevice overrides

    bool IsElementSupported(eInputElements elementId) const override;
    DigitalElementState GetDigitalElementState(eInputElements elementId) const override;
    AnalogElementState GetAnalogElementState(eInputElements elementId) const override;

private:
    explicit Keyboard(uint32 id);
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
