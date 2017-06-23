#include "Input/Mouse.h"

#include "Engine/Engine.h"
#include "Engine/Private/EngineBackend.h"
#include "Engine/Private/Dispatcher/MainDispatcherEvent.h"
#include "Input/InputElements.h"
#include "Input/InputSystem.h"
#include "Utils/StringFormat.h"

namespace DAVA
{
Mouse::Mouse(uint32 id)
    : InputDevice(id)
    , inputSystem(GetEngineContext()->inputSystem)
    , buttons{}
    , mousePosition{}
    , mouseWheelDelta{}
{
    Engine* engine = Engine::Instance();
    engine->endFrame.Connect(this, &Mouse::OnEndFrame);
    engine->PrimaryWindow()->focusChanged.Connect(this, &Mouse::OnWindowFocusChanged); // TODO: handle all the windows

    Private::EngineBackend::Instance()->InstallEventFilter(this, MakeFunction(this, &Mouse::HandleEvent));
}

Mouse::~Mouse()
{
    Engine* engine = Engine::Instance();
    engine->endFrame.Disconnect(this);
    engine->PrimaryWindow()->focusChanged.Disconnect(this);

    Private::EngineBackend::Instance()->UninstallEventFilter(this);
}

DigitalElementState Mouse::GetLeftButtonState() const
{
    return GetDigitalElementState(eInputElements::MOUSE_LBUTTON);
}

DigitalElementState Mouse::GetRightButtonState() const
{
    return GetDigitalElementState(eInputElements::MOUSE_RBUTTON);
}

DigitalElementState Mouse::GetMiddleButtonState() const
{
    return GetDigitalElementState(eInputElements::MOUSE_MBUTTON);
}

AnalogElementState Mouse::GetPosition() const
{
    return GetAnalogElementState(eInputElements::MOUSE_POSITION);
}

AnalogElementState Mouse::GetWheelDelta() const
{
    return GetAnalogElementState(eInputElements::MOUSE_WHEEL);
}

eInputElements Mouse::GetFirstPressedButton() const
{
    for (uint32 i = eInputElements::MOUSE_FIRST_BUTTON; i <= eInputElements::MOUSE_LAST_BUTTON; ++i)
    {
        if (buttons[i - eInputElements::MOUSE_FIRST_BUTTON].IsPressed())
        {
            return static_cast<eInputElements>(i);
        }
    }
    return eInputElements::NONE;
}

bool Mouse::IsElementSupported(eInputElements elementId) const
{
    return IsMouseInputElement(elementId);
}

DigitalElementState Mouse::GetDigitalElementState(eInputElements elementId) const
{
    DVASSERT(IsMouseButtonInputElement(elementId));
    return buttons[elementId - eInputElements::MOUSE_FIRST_BUTTON];
}

AnalogElementState Mouse::GetAnalogElementState(eInputElements elementId) const
{
    switch (elementId)
    {
    case eInputElements::MOUSE_WHEEL:
        return mouseWheelDelta;
    case eInputElements::MOUSE_POSITION:
        return mousePosition;
    default:
        DVASSERT(false, Format("Invalid element id passed to Mouse::GetAnalogElementState: %u", static_cast<uint32>(elementId)).c_str());
        return AnalogElementState();
    }
}

void Mouse::OnEndFrame()
{
    // Promote JustPressed & JustReleased states to Pressed/Released accordingly
    for (DigitalElementState& buttonState : buttons)
    {
        buttonState.OnEndFrame();
    }

    mouseWheelDelta.x = 0.f;
    mouseWheelDelta.y = 0.f;
}

void Mouse::OnWindowFocusChanged(DAVA::Window* window, bool focused)
{
    // Reset mouse state when window is unfocused
    if (!focused)
    {
        ResetState(window);
    }
}

bool Mouse::HandleEvent(const Private::MainDispatcherEvent& e)
{
    using Private::MainDispatcherEvent;

    bool isHandled = true;
    switch (e.type)
    {
    case MainDispatcherEvent::MOUSE_MOVE:
        HandleMouseMove(e);
        break;
    case MainDispatcherEvent::MOUSE_BUTTON_DOWN:
    case MainDispatcherEvent::MOUSE_BUTTON_UP:
        HandleMouseClick(e);
        break;
    case MainDispatcherEvent::MOUSE_WHEEL:
        HandleMouseWheel(e);
        break;
    default:
        isHandled = false;
        break;
    }
    return isHandled;
}

void Mouse::ResetState(Window* window)
{
    int64 timestamp = SystemTimer::GetMs();
    for (size_t i = 0; i < INPUT_ELEMENTS_MOUSE_BUTTON_COUNT; ++i)
    {
        DigitalElementState& buttonState = buttons[i];
        if (buttonState.IsPressed())
        {
            buttonState.Release();

            // Generate release event
            eInputElements elementId = static_cast<eInputElements>(eInputElements::MOUSE_FIRST_BUTTON + i);
            CreateAndSendButtonInputEvent(elementId,
                                          buttonState,
                                          window,
                                          timestamp,
                                          false);
        }
    }

    mouseWheelDelta.x = 0.f;
    mouseWheelDelta.y = 0.f;
}

void Mouse::HandleMouseClick(const Private::MainDispatcherEvent& e)
{
    bool pressed = e.type == Private::MainDispatcherEvent::MOUSE_BUTTON_DOWN;
    eMouseButtons button = e.mouseEvent.button;

    uint32 index = static_cast<uint32>(button) - 1;
    DigitalElementState& buttonState = buttons[index];
    pressed ? buttonState.Press() : buttonState.Release();

    mousePosition.x = e.mouseEvent.x;
    mousePosition.y = e.mouseEvent.y;

    CreateAndSendButtonInputEvent(static_cast<eInputElements>(eInputElements::MOUSE_FIRST_BUTTON + index),
                                  buttonState,
                                  e.window,
                                  e.timestamp / 1000.0f,
                                  e.mouseEvent.isRelative);
}

void Mouse::HandleMouseWheel(const Private::MainDispatcherEvent& e)
{
    InputEvent inputEvent;
    inputEvent.window = e.window;
    inputEvent.timestamp = e.timestamp / 1000.0f;
    inputEvent.deviceType = eInputDeviceTypes::MOUSE;
    inputEvent.device = this;
    inputEvent.elementId = eInputElements::MOUSE_WHEEL;
    inputEvent.analogState.x = e.mouseEvent.scrollDeltaX;
    inputEvent.analogState.y = e.mouseEvent.scrollDeltaY;
    inputEvent.analogState.z = 0.f;
    inputEvent.mouseEvent.isRelative = e.mouseEvent.isRelative;

    mousePosition.x = e.mouseEvent.x;
    mousePosition.y = e.mouseEvent.y;

    mouseWheelDelta.x = e.mouseEvent.scrollDeltaX;
    mouseWheelDelta.y = e.mouseEvent.scrollDeltaY;

    inputSystem->DispatchInputEvent(inputEvent);
}

void Mouse::HandleMouseMove(const Private::MainDispatcherEvent& e)
{
    InputEvent inputEvent;
    inputEvent.window = e.window;
    inputEvent.timestamp = e.timestamp / 1000.0f;
    inputEvent.deviceType = eInputDeviceTypes::MOUSE;
    inputEvent.device = this;
    inputEvent.elementId = eInputElements::MOUSE_POSITION;
    inputEvent.analogState.x = e.mouseEvent.x;
    inputEvent.analogState.y = e.mouseEvent.y;
    inputEvent.analogState.z = 0.f;
    inputEvent.mouseEvent.isRelative = e.mouseEvent.isRelative;

    mousePosition.x = e.mouseEvent.x;
    mousePosition.y = e.mouseEvent.y;

    inputSystem->DispatchInputEvent(inputEvent);
}

void Mouse::CreateAndSendButtonInputEvent(eInputElements elementId, DigitalElementState state, Window* window, int64 timestamp, bool isRelative)
{
    InputEvent inputEvent;
    inputEvent.window = window;
    inputEvent.timestamp = timestamp;
    inputEvent.deviceType = eInputDeviceTypes::MOUSE;
    inputEvent.device = this;
    inputEvent.mouseEvent.isRelative = isRelative;
    inputEvent.digitalState = state;
    inputEvent.elementId = elementId;

    inputSystem->DispatchInputEvent(inputEvent);
}

} // namespace DAVA
