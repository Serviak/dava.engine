#include "UnitTests/UnitTests.h"

#include <Engine/Engine.h>
#include <Engine/Private/EngineBackend.h>
#include <Engine/Private/Dispatcher/MainDispatcher.h>
#include <Engine/Private/Dispatcher/MainDispatcherEvent.h>
#include <DeviceManager/DeviceManager.h>
#include <Input/ActionSystem.h>
#include <Input/Keyboard.h>
#include <Input/Mouse.h>
#include <Input/TouchScreen.h>
#include <Logger/Logger.h>

using namespace DAVA;

DAVA_TESTCLASS (ActionSystemTestClass)
{
    void OnActionTriggered(Action action)
    {
        lastTriggeredAction.reset(new Action(action));
        ++triggeredActionsCounter;
    }

    void ResetTriggeredActionsInfo()
    {
        lastTriggeredAction.reset();
        triggeredActionsCounter = 0;
    }

    DAVA_TEST (ActionSystemActionTriggerDigitalTest)
    {
        // Check different combinations of digital elements that should trigger an action

        GetPrimaryWindow()->SetInputHandlingMode(eInputHandlingModes::HANDLE_ALWAYS);

        Keyboard* kb = GetEngineContext()->deviceManager->GetKeyboard();
        if (kb == nullptr)
        {
            Logger::Info("Skipping ActionSystemActionTriggerDigitalTest since there is no keyboard device");
            return;
        }

        ActionSystem* actionSystem = GetEngineContext()->actionSystem;
        actionSystem->ActionTriggered.Connect(this, &ActionSystemTestClass::OnActionTriggered);

        // Bind test set

        ActionSet set;

        // W, triggered continuously
        DigitalBinding action1;
        action1.actionId = ACTION_1;
        action1.digitalElements[0] = eInputElements::KB_W;
        action1.digitalStates[0] = DigitalElementState::Pressed();
        set.digitalBindings.push_back(action1);

        // Space, triggered once
        DigitalBinding action2;
        action2.actionId = ACTION_2;
        action2.digitalElements[0] = eInputElements::KB_SPACE;
        action2.digitalStates[0] = DigitalElementState::JustPressed();
        set.digitalBindings.push_back(action2);

        // LCtrl + Space, triggered once
        DigitalBinding action3;
        action3.actionId = ACTION_3;
        action3.digitalElements[0] = eInputElements::KB_LCTRL;
        action3.digitalStates[0] = DigitalElementState::Pressed();
        action3.digitalElements[1] = eInputElements::KB_SPACE;
        action3.digitalStates[1] = DigitalElementState::JustPressed();
        set.digitalBindings.push_back(action3);

        // RCtrl + Space, triggered continuously
        DigitalBinding action4;
        action4.actionId = ACTION_4;
        action4.digitalElements[0] = eInputElements::KB_RCTRL;
        action4.digitalStates[0] = DigitalElementState::Pressed();
        action4.digitalElements[1] = eInputElements::KB_SPACE;
        action4.digitalStates[1] = DigitalElementState::Pressed();
        set.digitalBindings.push_back(action4);

        actionSystem->BindSet(set, kb->GetId());

        // Check action1 (W Pressed)
        {
            // Press W, action1 should be triggered once
            SendKeyboardKeyDown(kb, eInputElements::KB_W);
            TEST_VERIFY(triggeredActionsCounter == 1);
            TEST_VERIFY(lastTriggeredAction != nullptr);
            TEST_VERIFY(lastTriggeredAction->actionId == ACTION_1);
            TEST_VERIFY(lastTriggeredAction->triggeredDevice == kb);

            // Press W, action1 should be triggered again
            SendKeyboardKeyDown(kb, eInputElements::KB_W);
            TEST_VERIFY(triggeredActionsCounter == 2);
            TEST_VERIFY(lastTriggeredAction != nullptr);
            TEST_VERIFY(lastTriggeredAction->actionId == ACTION_1);
            TEST_VERIFY(lastTriggeredAction->triggeredDevice == kb);

            // Press A, no action should be triggered
            SendKeyboardKeyDown(kb, eInputElements::KB_A);
            TEST_VERIFY(triggeredActionsCounter == 2);

            // Release A, no action should be triggered
            SendKeyboardKeyUp(kb, eInputElements::KB_A);
            TEST_VERIFY(triggeredActionsCounter == 2);

            // Release W, no action should be triggered
            SendKeyboardKeyUp(kb, eInputElements::KB_W);
            TEST_VERIFY(triggeredActionsCounter == 2);

            ResetTriggeredActionsInfo();
        }

        // Check action2 (Space JustPressed)
        {
            // Press Space, action2 should be triggered once
            SendKeyboardKeyDown(kb, eInputElements::KB_SPACE);
            TEST_VERIFY(triggeredActionsCounter == 1);
            TEST_VERIFY(lastTriggeredAction != nullptr);
            TEST_VERIFY(lastTriggeredAction->actionId == ACTION_2);
            TEST_VERIFY(lastTriggeredAction->triggeredDevice == kb);

            // Press Space, no action should be triggered
            SendKeyboardKeyDown(kb, eInputElements::KB_SPACE);
            TEST_VERIFY(triggeredActionsCounter == 1);

            // Press R, no action should be triggered
            SendKeyboardKeyDown(kb, eInputElements::KB_R);
            TEST_VERIFY(triggeredActionsCounter == 1);

            // Release R, no action should be triggered
            SendKeyboardKeyUp(kb, eInputElements::KB_R);
            TEST_VERIFY(triggeredActionsCounter == 1);

            // Release Space, no action should be triggered
            SendKeyboardKeyUp(kb, eInputElements::KB_SPACE);
            TEST_VERIFY(triggeredActionsCounter == 1);

            ResetTriggeredActionsInfo();
        }

        // Check action3 (LCtrl Pressed, Space JustPressed)
        {
            // Press LCtrl and Space, action3 should be triggered once
            SendKeyboardKeyDown(kb, eInputElements::KB_LCTRL);
            SendKeyboardKeyDown(kb, eInputElements::KB_SPACE);
            TEST_VERIFY(triggeredActionsCounter == 1);
            TEST_VERIFY(lastTriggeredAction != nullptr);
            TEST_VERIFY(lastTriggeredAction->actionId == ACTION_3);
            TEST_VERIFY(lastTriggeredAction->triggeredDevice == kb);

            // Press Space, no action should be triggered
            SendKeyboardKeyDown(kb, eInputElements::KB_SPACE);
            TEST_VERIFY(triggeredActionsCounter == 1);

            // Press LCtrl, no action should be triggered
            SendKeyboardKeyDown(kb, eInputElements::KB_LCTRL);
            TEST_VERIFY(triggeredActionsCounter == 1);

            // Release LCtrl, no action should be triggered
            SendKeyboardKeyUp(kb, eInputElements::KB_LCTRL);
            TEST_VERIFY(triggeredActionsCounter == 1);

            // Release Space, no action should be triggered
            SendKeyboardKeyUp(kb, eInputElements::KB_SPACE);
            TEST_VERIFY(triggeredActionsCounter == 1);

            ResetTriggeredActionsInfo();
        }

        // Check action4 (RCtrl Pressed, Space Pressed)
        {
            // Press RCtrl and Space, action4 should be triggered once
            SendKeyboardKeyDown(kb, eInputElements::KB_RCTRL);
            SendKeyboardKeyDown(kb, eInputElements::KB_SPACE);
            TEST_VERIFY(triggeredActionsCounter == 1);
            TEST_VERIFY(lastTriggeredAction != nullptr);
            TEST_VERIFY(lastTriggeredAction->actionId == ACTION_4);
            TEST_VERIFY(lastTriggeredAction->triggeredDevice == kb);

            // Press Space, action4 should be triggered once again
            SendKeyboardKeyDown(kb, eInputElements::KB_SPACE);
            TEST_VERIFY(triggeredActionsCounter == 2);
            TEST_VERIFY(lastTriggeredAction != nullptr);
            TEST_VERIFY(lastTriggeredAction->actionId == ACTION_4);
            TEST_VERIFY(lastTriggeredAction->triggeredDevice == kb);

            // Press RCtrl, action4 should be triggered once again
            SendKeyboardKeyDown(kb, eInputElements::KB_RCTRL);
            TEST_VERIFY(triggeredActionsCounter == 3);
            TEST_VERIFY(lastTriggeredAction != nullptr);
            TEST_VERIFY(lastTriggeredAction->actionId == ACTION_4);
            TEST_VERIFY(lastTriggeredAction->triggeredDevice == kb);

            // Press RAlt, no action should be triggered
            SendKeyboardKeyDown(kb, eInputElements::KB_RALT);
            TEST_VERIFY(triggeredActionsCounter == 3);

            // Release RAlt, no action should be triggered
            SendKeyboardKeyUp(kb, eInputElements::KB_RALT);
            TEST_VERIFY(triggeredActionsCounter == 3);

            // Release RCtrl, no action should be triggered
            SendKeyboardKeyUp(kb, eInputElements::KB_RCTRL);
            TEST_VERIFY(triggeredActionsCounter == 3);

            // Release Space, no action should be triggered
            SendKeyboardKeyUp(kb, eInputElements::KB_SPACE);
            TEST_VERIFY(triggeredActionsCounter == 3);

            ResetTriggeredActionsInfo();
        }

        actionSystem->UnbindAllSets();
        actionSystem->ActionTriggered.Disconnect(this);

        GetPrimaryWindow()->SetInputHandlingMode(eInputHandlingModes::HANDLE_ONLY_WHEN_FOCUSED);
    }

    DAVA_TEST (ActionSystemActionTriggeredDeviceTest)
    {
        // Check that triggeredDevice in triggered action is correct
        // and points to a device whose event has triggered the action

        GetPrimaryWindow()->SetInputHandlingMode(eInputHandlingModes::HANDLE_ALWAYS);

        Keyboard* kb = GetEngineContext()->deviceManager->GetKeyboard();
        if (kb == nullptr)
        {
            Logger::Info("Skipping ActionSystemActionTriggeredDeviceTest since there is no keyboard device");
            return;
        }

        Mouse* mouse = GetEngineContext()->deviceManager->GetMouse();
        if (mouse == nullptr)
        {
            Logger::Info("Skipping ActionSystemActionTriggeredDeviceTest since there is no mouse device");
            return;
        }

        ActionSystem* actionSystem = GetEngineContext()->actionSystem;
        actionSystem->ActionTriggered.Connect(this, &ActionSystemTestClass::OnActionTriggered);

        ActionSet set;

        // LCtrl Pressed + Mouse LButton JustPressed
        DigitalBinding action1;
        action1.actionId = ACTION_1;
        action1.digitalElements[0] = eInputElements::KB_LCTRL;
        action1.digitalStates[0] = DigitalElementState::Pressed();
        action1.digitalElements[1] = eInputElements::MOUSE_LBUTTON;
        action1.digitalStates[1] = DigitalElementState::JustPressed();
        set.digitalBindings.push_back(action1);

        actionSystem->BindSet(set, kb->GetId(), mouse->GetId());

        {
            // Press LCtrl and Mouse LButton, action1 should be triggered once, mouse should be a trigger
            SendKeyboardKeyDown(kb, eInputElements::KB_LCTRL);
            SendMouseButtonDown(mouse, eInputElements::MOUSE_LBUTTON);
            TEST_VERIFY(triggeredActionsCounter == 1);
            TEST_VERIFY(lastTriggeredAction != nullptr);
            TEST_VERIFY(lastTriggeredAction->actionId == ACTION_1);
            TEST_VERIFY(lastTriggeredAction->triggeredDevice == mouse);
            // Release buttons - no action should be fired
            SendKeyboardKeyUp(kb, eInputElements::KB_LCTRL);
            SendMouseButtonUp(mouse, eInputElements::MOUSE_LBUTTON);
            TEST_VERIFY(triggeredActionsCounter == 1);

            ResetTriggeredActionsInfo();
        }

        {
            // Press Mouse LButton and LCtrl, action1 should be triggered once, keyboard should be a trigger
            SendMouseButtonDown(mouse, eInputElements::MOUSE_LBUTTON);
            SendKeyboardKeyDown(kb, eInputElements::KB_LCTRL);
            TEST_VERIFY(triggeredActionsCounter == 1);
            TEST_VERIFY(lastTriggeredAction != nullptr);
            TEST_VERIFY(lastTriggeredAction->actionId == ACTION_1);
            TEST_VERIFY(lastTriggeredAction->triggeredDevice == kb);

            // Release buttons - no action should be fired
            SendKeyboardKeyUp(kb, eInputElements::KB_LCTRL);
            SendMouseButtonUp(mouse, eInputElements::MOUSE_LBUTTON);
            TEST_VERIFY(triggeredActionsCounter == 1);

            ResetTriggeredActionsInfo();
        }

        actionSystem->UnbindAllSets();
        actionSystem->ActionTriggered.Disconnect(this);

        GetPrimaryWindow()->SetInputHandlingMode(eInputHandlingModes::HANDLE_ONLY_WHEN_FOCUSED);
    }

    DAVA_TEST (ActionSystemActionTriggerAnalogTest)
    {
        // Check different combinations of digital and analog elements that should trigger an action with analog data
        // Use mouse device for that

        GetPrimaryWindow()->SetInputHandlingMode(eInputHandlingModes::HANDLE_ALWAYS);

        ActionSystem* actionSystem = GetEngineContext()->actionSystem;

        Mouse* mouse = GetEngineContext()->deviceManager->GetMouse();
        if (mouse != nullptr)
        {
            actionSystem->ActionTriggered.Connect(this, &ActionSystemTestClass::OnActionTriggered);

            ActionSet set;

            // RButton Pressed + Mouse moved
            AnalogBinding action1;
            action1.actionId = ACTION_1;
            action1.digitalElements[0] = eInputElements::MOUSE_RBUTTON;
            action1.digitalStates[0] = DigitalElementState::Pressed();
            action1.analogElementId = eInputElements::MOUSE_POSITION;
            set.analogBindings.push_back(action1);

            actionSystem->BindSet(set, mouse->GetId());

            {
                // Press Mouse LButton and LCtrl, action1 should be triggered once, keyboard should be a trigger
                SendMouseButtonDown(mouse, eInputElements::MOUSE_RBUTTON);
                TEST_VERIFY(triggeredActionsCounter == 0);

                // Move mouse and check that action1 is triggered with correct positions

                float32 mousePosX = 412.421f;
                float32 mousePosY = 1.03f;
                SendMouseMove(mouse, mousePosX, mousePosY);
                TEST_VERIFY(triggeredActionsCounter == 1);
                TEST_VERIFY(lastTriggeredAction->analogState.x == mousePosX);
                TEST_VERIFY(lastTriggeredAction->analogState.y == mousePosY);

                mousePosX = 5.0f;
                mousePosY = 21.81f;
                SendMouseMove(mouse, mousePosX, mousePosY);
                TEST_VERIFY(triggeredActionsCounter == 2);
                TEST_VERIFY(lastTriggeredAction->analogState.x == mousePosX);
                TEST_VERIFY(lastTriggeredAction->analogState.y == mousePosY);

                mousePosX = 531.1f;
                mousePosY = 0.0f;
                SendMouseMove(mouse, mousePosX, mousePosY);
                TEST_VERIFY(triggeredActionsCounter == 3);
                TEST_VERIFY(lastTriggeredAction->analogState.x == mousePosX);
                TEST_VERIFY(lastTriggeredAction->analogState.y == mousePosY);

                mousePosX = 2.0f;
                mousePosY = -3.0f;
                SendMouseMove(mouse, mousePosX, mousePosY, true);
                TEST_VERIFY(triggeredActionsCounter == 4);
                TEST_VERIFY(lastTriggeredAction->analogState.x == mousePosX);
                TEST_VERIFY(lastTriggeredAction->analogState.y == mousePosY);

                // Release buttons - no action should be fired
                SendMouseButtonUp(mouse, eInputElements::MOUSE_RBUTTON);
                TEST_VERIFY(triggeredActionsCounter == 4);

                ResetTriggeredActionsInfo();
            }

            actionSystem->UnbindAllSets();
            actionSystem->ActionTriggered.Disconnect(this);
        }
        else
        {
            Logger::Info("Skipping ActionSystemActionTriggerAnalogTest's mouse part since there is no mouse device");
        }

        GetPrimaryWindow()->SetInputHandlingMode(eInputHandlingModes::HANDLE_ONLY_WHEN_FOCUSED);
    }

    DAVA_TEST (ActionSystemActionTriggerDigitalAnalogStateTest)
    {
        // Check that digital binding correctly outputs overrided analog state

        GetPrimaryWindow()->SetInputHandlingMode(eInputHandlingModes::HANDLE_ALWAYS);

        Keyboard* kb = GetEngineContext()->deviceManager->GetKeyboard();
        if (kb == nullptr)
        {
            Logger::Info("Skipping ActionSystemActionTriggerDigitalAnalogStateTest since there is no keyboard device");
            return;
        }

        ActionSystem* actionSystem = GetEngineContext()->actionSystem;
        actionSystem->ActionTriggered.Connect(this, &ActionSystemTestClass::OnActionTriggered);

        // Bind test set

        ActionSet set;

        const AnalogElementState overridenAnalogState{ 14.3f, -3.04f, 12.12f };

        // W, triggered continuously, analog data overriden
        DigitalBinding action1;
        action1.actionId = ACTION_1;
        action1.digitalElements[0] = eInputElements::KB_W;
        action1.digitalStates[0] = DigitalElementState::Pressed();
        action1.outputAnalogState = overridenAnalogState;
        set.digitalBindings.push_back(action1);

        actionSystem->BindSet(set, kb->GetId());

        // Check action1 (W Pressed)
        {
            // Press W, action1 should be triggered once, analog data should be equal to outputAnalogState
            SendKeyboardKeyDown(kb, eInputElements::KB_W);
            TEST_VERIFY(triggeredActionsCounter == 1);
            TEST_VERIFY(lastTriggeredAction != nullptr);
            TEST_VERIFY(lastTriggeredAction->actionId == ACTION_1);
            TEST_VERIFY(lastTriggeredAction->triggeredDevice == kb);
            TEST_VERIFY(lastTriggeredAction->analogState.x == overridenAnalogState.x);
            TEST_VERIFY(lastTriggeredAction->analogState.y == overridenAnalogState.y);
            TEST_VERIFY(lastTriggeredAction->analogState.z == overridenAnalogState.z);

            // Release W, no action should be triggered
            SendKeyboardKeyUp(kb, eInputElements::KB_W);
            TEST_VERIFY(triggeredActionsCounter == 1);

            ResetTriggeredActionsInfo();
        }

        actionSystem->UnbindAllSets();
        actionSystem->ActionTriggered.Disconnect(this);

        GetPrimaryWindow()->SetInputHandlingMode(eInputHandlingModes::HANDLE_ONLY_WHEN_FOCUSED);
    }

    void SendKeyboardKeyDown(Keyboard * kb, eInputElements key)
    {
        using namespace DAVA::Private;

        Window* primaryWindow = GetPrimaryWindow();
        MainDispatcher* dispatcher = EngineBackend::Instance()->GetDispatcher();

        dispatcher->SendEvent(MainDispatcherEvent::CreateWindowKeyPressEvent(primaryWindow, MainDispatcherEvent::KEY_DOWN, kb->GetKeyNativeScancode(key), kb->GetKeyNativeScancode(key), DAVA::eModifierKeys::NONE, false));
    }

    void SendKeyboardKeyUp(Keyboard * kb, eInputElements key)
    {
        using namespace DAVA::Private;

        Window* primaryWindow = GetPrimaryWindow();
        MainDispatcher* dispatcher = EngineBackend::Instance()->GetDispatcher();

        dispatcher->SendEvent(MainDispatcherEvent::CreateWindowKeyPressEvent(primaryWindow, MainDispatcherEvent::KEY_UP, kb->GetKeyNativeScancode(key), kb->GetKeyNativeScancode(key), DAVA::eModifierKeys::NONE, false));
    }

    void SendMouseButtonDown(Mouse * mouse, eInputElements button)
    {
        using namespace DAVA::Private;

        Window* primaryWindow = GetPrimaryWindow();
        MainDispatcher* dispatcher = EngineBackend::Instance()->GetDispatcher();

        dispatcher->SendEvent(MainDispatcherEvent::CreateWindowMouseClickEvent(primaryWindow, MainDispatcherEvent::MOUSE_BUTTON_DOWN, static_cast<eMouseButtons>(button - eInputElements::MOUSE_FIRST_BUTTON + 1), 0.0f, 0.0f, 0, DAVA::eModifierKeys::NONE, false));
    }

    void SendMouseButtonUp(Mouse * mouse, eInputElements button)
    {
        using namespace DAVA::Private;

        Window* primaryWindow = GetPrimaryWindow();
        MainDispatcher* dispatcher = EngineBackend::Instance()->GetDispatcher();

        dispatcher->SendEvent(MainDispatcherEvent::CreateWindowMouseClickEvent(primaryWindow, MainDispatcherEvent::MOUSE_BUTTON_UP, static_cast<eMouseButtons>(button - eInputElements::MOUSE_FIRST_BUTTON + 1), 0.0f, 0.0f, 0, DAVA::eModifierKeys::NONE, false));
    }

    void SendMouseMove(Mouse * mouse, float toX, float toY, bool relative = false)
    {
        using namespace DAVA::Private;

        Window* primaryWindow = GetPrimaryWindow();
        MainDispatcher* dispatcher = EngineBackend::Instance()->GetDispatcher();

        dispatcher->SendEvent(MainDispatcherEvent::CreateWindowMouseMoveEvent(primaryWindow, toX, toY, eModifierKeys::NONE, relative));
    }

private:
    std::shared_ptr<Action> lastTriggeredAction;
    int triggeredActionsCounter = 0;

    const FastName ACTION_1 = FastName("Action 1");
    const FastName ACTION_2 = FastName("Action 2");
    const FastName ACTION_3 = FastName("Action 3");
    const FastName ACTION_4 = FastName("Action 4");
};
