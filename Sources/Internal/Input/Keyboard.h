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

    Some additional notes concerning keyboard handling on different platforms:
        - On Windows, KB_PRINTSCREEN will only send InputEvent with Released state and no Pressed state
        - On macOS, KB_CAPSLOCK works like a toggle: that is, it's state is pressed when it's toggled and released when it's untoggled
        - On Android, keyboard elements represent virtual keys and not scancodes like on other platforms (i.e. KB_W is sent on QWERTY keyboard when 'W' is pressed and on AZERTY keyboard when 'Z' is pressed)
*/
class Keyboard final : public InputDevice
{
    friend class DeviceManager; // For creation

    // For QT backend implementation
    // TODO: Left for backward compatibility, but probably should be removed
    friend class RenderWidgetOGL;
    friend class DavaQtApplyModifier;
    friend class DavaQtKeyboard;

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
    void OnWindowSizeChanged(DAVA::Window* window, Size2f, Size2f);

    void ResetState(Window* window);
    void OnKeyPressed(eInputElements element, Window* window, int64 timestamp);
    void OnKeyReleased(eInputElements element, Window* window, int64 timestamp);

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
