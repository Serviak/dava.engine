#include "Infrastructure/TestBed.h"
#include "Tests/KeyboardTest.h"

#include <DeviceManager/DeviceManager.h>
#include <Engine/Engine.h>
#include <Input/Gamepad.h>
#include <Input/Mouse.h>
#include <UI/Focus/UIFocusComponent.h>
#include <UI/Render/UIDebugRenderComponent.h>
#include <Render/2D/Sprite.h>

using namespace DAVA;

struct Finger
{
    int32 index = 0;
    UIControl* img = nullptr;
    bool isActive = false;
};

Array<Finger, 10> touches;
Vector2 hiddenPos(-100, -100);

auto gamepadButtonsNames =
{ "button_a", "button_b", "button_x", "button_y", "button_left", "button_right",
  "button_up", "button_down", "button_select", "button_start",
  "shift_left", "shift_right", "triger_left", "triger_right",
  "stick_left", "stick_right" };

Map<String, UIControl*> gamepadButtons;

Rect gamepadPos(500, 000, 800, 450);
float32 gamepadStickDeltaMove = 20.f; // 20 pixels

KeyboardTest::KeyboardTest(TestBed& app)
    : BaseScreen(app, "KeyboardTest")
    , app(app)
{
}

void KeyboardTest::LoadResources()
{
    BaseScreen::LoadResources();

    ScopedPtr<FTFont> font(FTFont::Create("~res:/Fonts/korinna.ttf"));

    previewText = new UIStaticText(Rect(20, 30, 400, 200));
    previewText->SetFont(font);
    previewText->SetTextColor(Color::White);
    previewText->SetMultiline(true);
    previewText->SetText(L"Press (Hold) and Unpress keys\nOn MacOS test gestures magnify/rotate/swipe");
    previewText->GetOrCreateComponent<UIDebugRenderComponent>();
    previewText->SetTextAlign(ALIGN_LEFT | ALIGN_TOP);
    AddControl(previewText);

    descriptionText = new UIStaticText(Rect(20, 230, 200, 400));
    descriptionText->GetOrCreateComponent<UIDebugRenderComponent>();
    descriptionText->SetTextColor(Color::White);
    descriptionText->SetFont(font);
    descriptionText->GetOrCreateComponent<UIFocusComponent>();
    UIControlBackground* descriptionTextBg = descriptionText->GetOrCreateComponent<UIControlBackground>();
    descriptionTextBg->SetAlign(ALIGN_LEFT | ALIGN_TOP);
    descriptionText->SetTextAlign(ALIGN_LEFT | ALIGN_TOP);
    descriptionText->SetMultiline(true);
    descriptionText->SetMultilineType(UIStaticText::MULTILINE_ENABLED_BY_SYMBOL);
    AddControl(descriptionText);

    InputSystem* inputSystem = app.GetEngine().GetContext()->inputSystem;
    gamepadInputToken = inputSystem->AddHandler(eInputDeviceTypes::CLASS_GAMEPAD, MakeFunction(this, &KeyboardTest::OnGamepadEvent));

    rawInputToken = inputSystem->AddHandler(eInputDeviceTypes::CLASS_ALL, MakeFunction(this, &KeyboardTest::InputEventHandler));

    resetButton = new UIButton(Rect(420, 30, 50, 30));
    resetButton->GetOrCreateComponent<UIDebugRenderComponent>();
    resetButton->SetStateFont(0xFF, font);
    resetButton->SetStateFontColor(0xFF, Color::White);
    resetButton->SetStateText(0xFF, L"Reset");
    resetButton->AddEvent(UIButton::EVENT_TOUCH_UP_INSIDE, Message(this, &KeyboardTest::OnResetClick));
    AddControl(resetButton);

    logEventsButton = new UIButton(Rect(480, 30, 50, 30));
    logEventsButton->SetDebugDraw(true);
    logEventsButton->SetStateFont(0xFF, font);
    logEventsButton->SetStateFontColor(0xFF, Color::White);
    logEventsButton->SetStateText(0xFF, L"Log");
    logEventsButton->AddEvent(UIButton::EVENT_TOUCH_UP_INSIDE, Message(this, &KeyboardTest::OnLogClick));
    AddControl(logEventsButton);

    for (auto& touch : touches)
    {
        touch.img = new UIButton(Rect(0, 0, 50, 50));
        touch.img->GetOrCreateComponent<UIDebugRenderComponent>();
        touch.img->SetInputEnabled(false);
        touch.img->SetPosition(hiddenPos);
        auto back = touch.img->GetBackground();
        int red = std::rand() % 256;
        int green = std::rand() % 256;
        int blue = std::rand() % 256;
        int color = 0xFF000000 | (blue << 16) | (green << 8) | (red);
        back->SetDrawColor(color);
        back->SetColor(color);
        back->SetDrawType(UIControlBackground::eDrawType::DRAW_FILL);

        AddControl(touch.img);
    }

    redBox = new UIButton(Rect(512, 512, 128, 128));
    redBox->SetPivotPoint(Vector2(64.f, 64.f));
    redBox->SetInputEnabled(false);
    redBox->GetOrCreateComponent<UIDebugRenderComponent>();
    auto boxBack = redBox->GetBackground();
    boxBack->SetDrawColor(Color(1.f, 0.f, 0.f, 1.f));
    boxBack->SetColor(Color(1.f, 0.f, 0.f, 1.f));
    boxBack->SetDrawType(UIControlBackground::eDrawType::DRAW_FILL);
    AddControl(redBox);

    gamepad = new UIControl(gamepadPos);
    FilePath pathToBack("~res:/TestData/GamepadTest/gamepad.png");
    ScopedPtr<Sprite> gamepadSprite(Sprite::CreateFromSourceFile(pathToBack));
    UIControlBackground* bg = gamepad->GetOrCreateComponent<UIControlBackground>();
    bg->SetModification(ESM_VFLIP | ESM_HFLIP);
    bg->SetSprite(gamepadSprite, 0);
    AddControl(gamepad);

    for (auto& buttonOrAxisName : gamepadButtonsNames)
    {
        UIControl* img = new UIControl(gamepadPos);
        auto path = FilePath("~res:/TestData/GamepadTest/") + buttonOrAxisName + ".png";
        UIControlBackground* bg = img->GetOrCreateComponent<UIControlBackground>();
        bg->SetModification(ESM_VFLIP | ESM_HFLIP);

        ScopedPtr<Sprite> sprite(Sprite::CreateFromSourceFile(path));
        bg->SetSprite(sprite, 0);
        gamepadButtons[buttonOrAxisName] = img;
        AddControl(img);
        img->SetVisibilityFlag(false);
    }
}

void KeyboardTest::UnloadResources()
{
    InputSystem* inputSystem = app.GetEngine().GetContext()->inputSystem;
    inputSystem->RemoveHandler(pointerInputToken);
    inputSystem->RemoveHandler(keyboardInputToken);
    inputSystem->RemoveHandler(gamepadInputToken);

    if (rawInputToken != 0)
    {
        inputSystem->RemoveHandler(rawInputToken);
        rawInputToken = 0;
    }
    if (logHandlerToken != 0)
    {
        inputSystem->RemoveHandler(logHandlerToken);
        logHandlerToken = 0;
    }

    SafeRelease(previewText);
    SafeRelease(descriptionText);
    SafeRelease(resetButton);
    SafeRelease(logEventsButton);
    SafeRelease(redBox);

    for (auto& touch : touches)
    {
        SafeRelease(touch.img);
    }

    SafeRelease(gamepad);

    for (auto& gamepadButton : gamepadButtons)
    {
        SafeRelease(gamepadButton.second);
    }

    BaseScreen::UnloadResources();
}

void KeyboardTest::OnResetClick(BaseObject* sender, void* data, void* callerData)
{
    ResetCounters();
    descriptionText->SetText(L"");
}

void KeyboardTest::OnLogClick(DAVA::BaseObject* sender, void* data, void* callerData)
{
    InputSystem* inputSystem = GetEngineContext()->inputSystem;
    if (logHandlerToken == 0)
    {
        Logger::Debug("Input event logging enabled");
        logHandlerToken = inputSystem->AddHandler(eInputDeviceTypes::CLASS_ALL, MakeFunction(this, &KeyboardTest::InputEventLogHandler));
    }
    else
    {
        Logger::Debug("Input event logging disabled");
        inputSystem->RemoveHandler(logHandlerToken);
        logHandlerToken = 0;
    }
}

void KeyboardTest::ResetCounters()
{
    numKeyboardEvents = 0;
    numKeyDown = 0;
    numKeyUp = 0;
    numKeyDownRepeat = 0;
    numChar = 0;
    numCharRepeat = 0;
    lastChar = L'\0';

    numMouseEvents = 0;
    numDrag = 0;
    numMouseMove = 0;
    numMouseDown = 0;
    numMouseUp = 0;
    numMouseWheel = 0;
    numMouseCancel = 0;
    numMouseDblUp = 0;
    numMouseDblDown = 0;
    lastMouseKey = L'\0';
    lastMouseX = 0;
    lastMouseY = 0;
    lastWheel = 0.f;

    Gamepad* gamepad = GetEngineContext()->deviceManager->GetGamepad();
    if (gamepad != nullptr)
    {
        for (uint32 i = eInputElements::GAMEPAD_FIRST; i <= eInputElements::GAMEPAD_LAST; ++i)
        {
            const InputElementInfo& info = GetInputElementInfo(static_cast<eInputElements>(i));
            bool elementSupported = gamepad->IsElementSupported(static_cast<eInputElements>(i));
            Logger::Debug("%s %s: %s", info.name.c_str(), info.type == eInputElementTypes::DIGITAL ? "[D]" : "[A]", elementSupported ? "yes" : "no");
        }
    }
}

bool KeyboardTest::InputEventLogHandler(const DAVA::InputEvent& inputEvent)
{
    const InputElementInfo& info = GetInputElementInfo(inputEvent.elementId);
    if (info.type == eInputElementTypes::DIGITAL)
    {
        String s;
        if (inputEvent.digitalState.IsPressed())
        {
            s = "PRESSED";
            if (inputEvent.digitalState.IsJustPressed())
            {
                s += "|JUST_PRESSED";
            }
        }
        else if (inputEvent.digitalState.IsReleased())
        {
            s = "RELEASED";
            if (inputEvent.digitalState.IsJustReleased())
            {
                s += "|JUST_RELEASED";
            }
        }
        Logger::Debug("============= %s [D]: 0x%X {%s}", info.name.c_str(), s.c_str());
    }
    else if (info.type == eInputElementTypes::ANALOG)
    {
        Logger::Debug("============= %s [A]: x=%f, y=%f, z=%f", info.name.c_str(), inputEvent.analogState.x, inputEvent.analogState.y, inputEvent.analogState.z);
    }
    return false;
}

bool KeyboardTest::InputEventHandler(const InputEvent& inputEvent)
{
    StringStream ss;
    if (IsMouseInputElement(inputEvent.elementId))
    {
        Mouse* mouse = GetEngineContext()->deviceManager->GetMouse();
        for (eInputElements i = eInputElements::MOUSE_FIRST_BUTTON; i <= eInputElements::MOUSE_LAST_BUTTON; i = static_cast<eInputElements>(i + 1))
        {
            DigitalElementState state = mouse->GetDigitalElementState(i);
            const InputElementInfo& info = GetInputElementInfo(static_cast<eInputElements>(i));
            ss << info.name << "=" << state.IsPressed() << std::endl;
        }
        AnalogElementState pos = mouse->GetAnalogElementState(eInputElements::MOUSE_POSITION);
        ss << GetInputElementInfo(eInputElements::MOUSE_POSITION).name << "=" << pos.x << ", " << pos.y << std::endl;
    }

    descriptionText->SetText(UTF8Utils::EncodeToWideString(ss.str()));
    return false;
}

bool KeyboardTest::OnGamepadEvent(const DAVA::InputEvent& e)
{
    //Logger::Info("gamepad tid: %2d, x: %.3f, y:%.3f", event->tid, event->point.x, event->point.y);

    DVASSERT(e.deviceType == eInputDevices::GAMEPAD);

    switch (e.elementId)
    {
    case eInputElements::GAMEPAD_A:
        UpdateGamepadElement("button_a", e.digitalState.IsPressed());
        break;
    case eInputElements::GAMEPAD_B:
        UpdateGamepadElement("button_b", e.digitalState.IsPressed());
        break;
    case eInputElements::GAMEPAD_X:
        UpdateGamepadElement("button_x", e.digitalState.IsPressed());
        break;
    case eInputElements::GAMEPAD_Y:
        UpdateGamepadElement("button_y", e.digitalState.IsPressed());
        break;
    case eInputElements::GAMEPAD_LSHOULDER:
        UpdateGamepadElement("shift_left", e.digitalState.IsPressed());
        break;
    case eInputElements::GAMEPAD_RSHOULDER:
        UpdateGamepadElement("shift_right", e.digitalState.IsPressed());
        break;
    case eInputElements::GAMEPAD_LTHUMB:
        UpdateGamepadElement("triger_left", e.digitalState.IsPressed());
        break;
    case eInputElements::GAMEPAD_RTHUMB:
        UpdateGamepadElement("triger_right", e.digitalState.IsPressed());
        break;
    case eInputElements::GAMEPAD_AXIS_LTHUMB:
        UpdateGamepadStickX("stick_left", e.analogState.x);
        break;
    case eInputElements::GAMEPAD_AXIS_RTHUMB:
        UpdateGamepadStickX("stick_right", e.analogState.x);
        break;
    case eInputElements::GAMEPAD_DPAD_LEFT:
        UpdateGamepadElement("button_left", e.digitalState.IsPressed());
        break;
    case eInputElements::GAMEPAD_DPAD_RIGHT:
        UpdateGamepadElement("button_right", e.digitalState.IsPressed());
        break;
    case eInputElements::GAMEPAD_DPAD_UP:
        UpdateGamepadElement("button_up", e.digitalState.IsPressed());
        break;
    case eInputElements::GAMEPAD_DPAD_DOWN:
        UpdateGamepadElement("button_down", e.digitalState.IsPressed());
        break;
    default:
        Logger::Error("not handled gamepad input event element: %d", static_cast<uint32>(e.elementId));
    }
    return false;
}

void KeyboardTest::OnGestureEvent(UIEvent* event)
{
    float32 magnification = event->gesture.magnification;
    Vector2 newSize = redBox->GetSize();
    if (magnification > -1.f && magnification < 1.f)
    {
        newSize.x *= (1.0f + magnification);
        newSize.y *= (1.0f + magnification);
    }
    redBox->SetSize(newSize);

    float32 angleDegrees = event->gesture.rotation;
    if (angleDegrees != 0.f)
    {
        angleDegrees *= -1.f;
    }
    float32 newAngle = redBox->GetAngle() + ((angleDegrees / 180) * 3.14f);
    redBox->SetAngle(newAngle);

    float swipeStep = 50.f;
    Vector2 newPos = redBox->GetPosition() +
    Vector2(swipeStep * event->gesture.dx,
            swipeStep * event->gesture.dy);
    redBox->SetPosition(newPos);
}

void KeyboardTest::UpdateGamepadElement(String name, bool isVisible)
{
    gamepadButtons[name]->SetVisibilityFlag(isVisible);
}

void KeyboardTest::UpdateGamepadStickX(String name, float axisValue)
{
    UIControl* ctrl = gamepadButtons[name];
    Vector2 pos = ctrl->GetPosition();
    if (std::abs(axisValue) >= 0.05f)
    {
        pos.x = gamepadPos.GetPosition().x + (axisValue * gamepadStickDeltaMove);
    }
    else
    {
        pos.x = gamepadPos.GetPosition().x;
    }
    ctrl->SetPosition(pos);
    UpdateGamepadElement(name, pos != gamepadPos.GetPosition());
}

void KeyboardTest::UpdateGamepadStickY(String name, float axisValue)
{
    UIControl* ctrl = gamepadButtons[name];
    Vector2 pos = ctrl->GetPosition();
    if (std::abs(axisValue) >= 0.05f)
    {
        pos.y = gamepadPos.GetPosition().y + (axisValue * gamepadStickDeltaMove * -1); // -1 y axis from up to down
    }
    else
    {
        pos.y = gamepadPos.GetPosition().y;
    }
    ctrl->SetPosition(pos);
    UpdateGamepadElement(name, pos != gamepadPos.GetPosition());
}
