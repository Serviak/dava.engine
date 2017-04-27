#include "UnitTests/UnitTests.h"

#include <Engine/Engine.h>
#include <Engine/Private/EngineBackend.h>
#include <Engine/Private/Dispatcher/MainDispatcher.h>
#include <Engine/Private/Dispatcher/MainDispatcherEvent.h>
#include <DeviceManager/DeviceManager.h>
#include <Input/Keyboard.h>
#include <Logger/Logger.h>

using namespace DAVA;

DAVA_TESTCLASS (KeyboardTestClass)
{
    DAVA_TEST (KeyboardSupportedElementsTest)
    {
        // Check keyboard supported elements:
        //   - all keyboard elements are supported
        //   - all non-keyboard elements are not supported

        Keyboard* kb = GetEngineContext()->deviceManager->GetKeyboard();
        if (kb == nullptr)
        {
            return;
        }

        for (uint32 i = static_cast<uint32>(eInputElements::FIRST); i <= static_cast<uint32>(eInputElements::LAST); ++i)
        {
            eInputElements element = static_cast<eInputElements>(i);
            TEST_VERIFY(IsKeyboardInputElement(element) ? kb->IsElementSupported(element) : !kb->IsElementSupported(element));
        }
    }

    DAVA_TEST (KeyboardDefaultStateTest)
    {
        // Check keyboard default state: equal to released for each key

        Keyboard* kb = GetEngineContext()->deviceManager->GetKeyboard();
        if (kb == nullptr)
        {
            return;
        }

        for (uint32 i = static_cast<uint32>(eInputElements::KB_FIRST); i <= static_cast<uint32>(eInputElements::KB_LAST); ++i)
        {
            eInputElements element = static_cast<eInputElements>(i);
            DigitalElementState state = kb->GetDigitalElementState(element);
            TEST_VERIFY(state.IsReleased() && !state.IsJustReleased());
        }
    }

    DAVA_TEST (KeyboardTranslateElementToWideStringTest)
    {
        // TODO
    }

    DAVA_TEST (KeyboardEventHandlingTest)
    {
        // Check event handling by the keyboard, for each key:
        //   - Check that initial state is released
        //   - Imitate platform sending KEY_DOWN event (it also tests `GetElementNativeScancode` method)
        //   - Check that state has changed to just pressed
        //   - Wait for the next frame, check it has changed to pressed
        //   - Imititate platform sending KEY_UP event
        //   - Check that state has changed to just released
        //   - Wait for the next frame, check it has changed to released
        //
        // Handled in Update()

        Keyboard* kb = GetEngineContext()->deviceManager->GetKeyboard();
        if (kb == nullptr)
        {
            keyboardEventHandlingTestFinished = true;
            return;
        }
    }

    DAVA_TEST (KeyboardMultipleKeysEventHandlingTest)
    {
        // TODO
    }

    void CheckSingleState(Keyboard * keyboard, eInputElements requiredElement, DigitalElementState requiredState)
    {
        // All elements should be in released state, `requiredElement` must be in `requiredState`
        for (uint32 i = static_cast<uint32>(eInputElements::KB_FIRST); i <= static_cast<uint32>(eInputElements::KB_LAST); ++i)
        {
            eInputElements element = static_cast<eInputElements>(i);
            DigitalElementState state = keyboard->GetDigitalElementState(element);

            if (element == requiredElement)
            {
                TEST_VERIFY(state == requiredState);
            }
            else
            {
                TEST_VERIFY(state.IsReleased() && !state.IsJustReleased());
            }
        }
    }

    void Update(float32 timeElapsed, const String& testName) override
    {
        if (testName == "KeyboardEventHandlingTest" && !keyboardEventHandlingTestFinished)
        {
            using namespace DAVA::Private;

            Keyboard* kb = GetEngineContext()->deviceManager->GetKeyboard();

            if (!waitingForFirstFrame && !waitingForSecondFrame)
            {
                // Check that button is released
                // Send KEY_DOWN and check that button is just pressed
                // Wait for the next frame

                Logger::Info("KeyboardEventHandlingTest: testing element '%s'", GetInputElementInfo(currentElement).name.c_str());

                DigitalElementState currentElementStateBeforeEvent = kb->GetDigitalElementState(currentElement);
                CheckSingleState(kb, currentElement, DigitalElementState::Released());

                Window* primaryWindow = GetPrimaryWindow();
                MainDispatcher* dispatcher = EngineBackend::Instance()->GetDispatcher();
                uint32 currentElementNativeScancode = kb->GetElementNativeScancode(currentElement);
                dispatcher->SendEvent(MainDispatcherEvent::CreateWindowKeyPressEvent(primaryWindow, MainDispatcherEvent::KEY_DOWN, currentElementNativeScancode, DAVA::eModifierKeys::NONE, false));

                DigitalElementState currentElementStateAfterEvent = kb->GetDigitalElementState(currentElement);
                CheckSingleState(kb, currentElement, DigitalElementState::JustPressed());

                waitingForFirstFrame = true;
            }
            else if (waitingForFirstFrame)
            {
                // Check that button is pressed
                // Send KEY_UP and check that button is just released
                // Wait for the next frame

                DigitalElementState currentElementStateBeforeEvent = kb->GetDigitalElementState(currentElement);
                CheckSingleState(kb, currentElement, DigitalElementState::Pressed());

                Window* primaryWindow = GetPrimaryWindow();
                MainDispatcher* dispatcher = EngineBackend::Instance()->GetDispatcher();
                dispatcher->SendEvent(MainDispatcherEvent::CreateWindowKeyPressEvent(primaryWindow, MainDispatcherEvent::KEY_UP, kb->GetElementNativeScancode(currentElement), DAVA::eModifierKeys::NONE, false));

                DigitalElementState currentElementStateAfterEvent = kb->GetDigitalElementState(currentElement);
                CheckSingleState(kb, currentElement, DigitalElementState::JustReleased());

                waitingForFirstFrame = false;
                waitingForSecondFrame = true;
            }
            else if (waitingForSecondFrame)
            {
                // Check that button is released
                // Go to the next element (if not finished yet)

                DigitalElementState currentElementState = kb->GetDigitalElementState(currentElement);
                CheckSingleState(kb, currentElement, DigitalElementState::Released());

                currentElement = static_cast<eInputElements>(static_cast<uint32>(currentElement) + 1);

#if defined(__DAVAENGINE_WINDOWS__)
                while (currentElement == eInputElements::KB_LCMD || currentElement == eInputElements::KB_RCMD)
                {
                    currentElement = static_cast<eInputElements>(static_cast<uint32>(currentElement) + 1);
                }
#elif defined(__DAVAENGINE_MACOS__)
                while (currentElement == eInputElements::KB_LWIN || currentElement == eInputElements::KB_RWIN)
                {
                    currentElement = static_cast<eInputElements>(static_cast<uint32>(currentElement) + 1);
                }
#endif

                if (currentElement > eInputElements::KB_LAST)
                {
                    keyboardEventHandlingTestFinished = true;
                }

                waitingForSecondFrame = false;
            }
        }
    }

    bool TestComplete(const String& testName) const override
    {
        if (testName != "KeyboardEventHandlingTest")
        {
            return true;
        }
        else
        {
            return keyboardEventHandlingTestFinished;
        }
    }

private:
    // KeyboardEventHandlingTest variables
    eInputElements currentElement = eInputElements::KB_FIRST;
    bool waitingForFirstFrame = false; // Button has been pressed, wait for the next frame to check that its status has changed
    bool waitingForSecondFrame = false; // Button has been released, wait for the next frame to check that its status has changed
    bool keyboardEventHandlingTestFinished = false;
};