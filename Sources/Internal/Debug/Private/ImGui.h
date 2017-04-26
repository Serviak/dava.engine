#pragma once

#include <imgui/imgui.h>

namespace DAVA
{
struct InputEvent;
}

namespace ImGui
{
void Initialize();
bool IsInitialized();
void OnFrameBegin();
void OnFrameEnd();
bool OnInput(const DAVA::InputEvent& input);
void Uninitialize();
}
