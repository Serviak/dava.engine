#pragma once

#include "Infrastructure/BaseScreen.h"
#include "Input/KeyboardInputDevice.h"
#include "Input/ActionSystem.h"

#include <map>

class TestBed;
class InputSystemTest : public BaseScreen
{
public:
    InputSystemTest(TestBed& app);

protected:
    void LoadResources() override;
    void UnloadResources() override;

private:
    DAVA::UIButton* CreateKeyboardUIButton(DAVA::eKeyboardKey key, DAVA::WideString text, DAVA::FTFont* font, DAVA::float32 x, DAVA::float32 y, DAVA::float32 w, DAVA::float32 h);
    bool OnInputEvent(DAVA::InputEvent const& event);
    void OnBeginFrame();
    void OnAction(DAVA::Action action);

    void CreateKeyboardUI();
    void CreateMouseUI();
    void CreateActionsUI();

    std::unordered_map<DAVA::uint32, DAVA::UIButton*> keyboardButtons;
    std::unordered_map<DAVA::uint32, DAVA::UIButton*> mouseButtons;
    std::unordered_map<DAVA::uint32, DAVA::UIStaticText*> actionCounters;
};
