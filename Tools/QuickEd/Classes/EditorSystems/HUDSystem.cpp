/*==================================================================================
Copyright (c) 2008, binaryzebra
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.
* Neither the name of the binaryzebra nor the
names of its contributors may be used to endorse or promote products
derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/

#include "HUDSystem.h"

#include "UI/UIControl.h"
#include "UI/UIEvent.h"
#include "Base/BaseTypes.h"

#include "Model/PackageHierarchy/ControlNode.h"
#include "Model/PackageHierarchy/PackageNode.h"
#include "Model/PackageHierarchy/PackageControlsNode.h"

#include "EditorSystems/HUDControls.h"
#include "EditorSystems/KeyboardProxy.h"

using namespace DAVA;

namespace
{
const Array<HUDAreaInfo::eArea, 2> AreasToHide = { { HUDAreaInfo::PIVOT_POINT_AREA, HUDAreaInfo::ROTATE_AREA } };
}

ControlContainer* CreateControlContainer(HUDAreaInfo::eArea area)
{
    switch (area)
    {
    case HUDAreaInfo::PIVOT_POINT_AREA:
        return new PivotPointControl();
    case HUDAreaInfo::ROTATE_AREA:
        return new RotateControl();
    case HUDAreaInfo::TOP_LEFT_AREA:
    case HUDAreaInfo::TOP_CENTER_AREA:
    case HUDAreaInfo::TOP_RIGHT_AREA:
    case HUDAreaInfo::CENTER_LEFT_AREA:
    case HUDAreaInfo::CENTER_RIGHT_AREA:
    case HUDAreaInfo::BOTTOM_LEFT_AREA:
    case HUDAreaInfo::BOTTOM_CENTER_AREA:
    case HUDAreaInfo::BOTTOM_RIGHT_AREA:
        return new FrameRectControl(area);
    case HUDAreaInfo::FRAME_AREA:
        return CreateContainerWithBorders<FrameControl>();
    default:
        DVASSERT(!"unacceptable value of area");
        return nullptr;
    }
}

struct HUDSystem::HUD
{
    HUD(ControlNode* node, UIControl* hudControl);
    ~HUD();
    ControlNode* node = nullptr;
    UIControl* control = nullptr;
    UIControl* hudControl = nullptr;
    RefPtr<HUDContainer> container;
    Map<HUDAreaInfo::eArea, RefPtr<ControlContainer>> hudControls;
};

HUDSystem::HUD::HUD(ControlNode* node_, UIControl* hudControl_)
    : node(node_)
    , control(node_->GetControl())
    , hudControl(hudControl_)
    , container(new HUDContainer(control))
{
    container->SetName("Container for HUD controls of node " + node_->GetName());
    DAVA::Vector<HUDAreaInfo::eArea> areas(HUDAreaInfo::AREAS_COUNT);
    int begin = HUDAreaInfo::AREAS_BEGIN;
    if (node->GetParent() != nullptr && node->GetParent()->GetControl() != nullptr)
    {
        std::generate(areas.begin(), areas.end(), [begin]() mutable {
            return static_cast<HUDAreaInfo::eArea>(begin++);
        });
    }
    else
    {
        //custom areas
        areas.push_back(HUDAreaInfo::TOP_LEFT_AREA);
        areas.push_back(HUDAreaInfo::TOP_CENTER_AREA);
        areas.push_back(HUDAreaInfo::TOP_RIGHT_AREA);
        areas.push_back(HUDAreaInfo::CENTER_LEFT_AREA);
        areas.push_back(HUDAreaInfo::CENTER_RIGHT_AREA);
        areas.push_back(HUDAreaInfo::BOTTOM_LEFT_AREA);
        areas.push_back(HUDAreaInfo::BOTTOM_CENTER_AREA);
        areas.push_back(HUDAreaInfo::BOTTOM_RIGHT_AREA);
        areas.push_back(HUDAreaInfo::FRAME_AREA);
    }
    for (HUDAreaInfo::eArea area : areas)
    {
        ControlContainer* controlContainer = CreateControlContainer(area);
        container->AddChild(controlContainer);
        hudControls[area] = controlContainer;
    }
    hudControl->AddControl(container.Get());
    container->InitFromGD(control->GetGeometricData());
}

HUDSystem::HUD::~HUD()
{
    hudControl->RemoveControl(container.Get());
}

HUDSystem::HUDSystem(EditorSystemsManager* parent)
    : BaseEditorSystem(parent)
    , hudControl(new UIControl())
    , selectionRectControl(CreateContainerWithBorders<SelectionRect>())
    , sortedControlList(CompareByLCA)
{
    InvalidatePressedPoint();
    systemManager->GetRootControl()->AddControl(hudControl.Get());
    hudControl->AddControl(selectionRectControl.Get());
    hudControl->SetName("hudControl");
    systemManager->SelectionChanged.Connect(this, &HUDSystem::OnSelectionChanged);
    systemManager->EmulationModeChangedSignal.Connect(this, &HUDSystem::OnEmulationModeChanged);
    systemManager->EditingRootControlsChanged.Connect(this, &HUDSystem::OnRootContolsChanged);
    systemManager->MagnetLinesChanged.Connect(this, &HUDSystem::OnMagnetLinesChanged);
}

HUDSystem::~HUDSystem()
{
    systemManager->GetRootControl()->RemoveControl(hudControl.Get());
}

void HUDSystem::OnSelectionChanged(const SelectedNodes& selected, const SelectedNodes& deselected)
{
    selectionContainer.MergeSelection(selected, deselected);
    for (auto node : deselected)
    {
        ControlNode* controlNode = dynamic_cast<ControlNode*>(node);
        if (nullptr != controlNode)
        {
            hudMap.erase(controlNode);
            sortedControlList.erase(controlNode);
        }
    }

    for (auto node : selected)
    {
        ControlNode* controlNode = dynamic_cast<ControlNode*>(node);
        if (controlNode != nullptr)
        {
            if (nullptr != controlNode && nullptr != controlNode->GetControl())
            {
                hudMap[controlNode] = std::make_unique<HUD>(controlNode, hudControl.Get());
                sortedControlList.insert(controlNode);
            }
        }
    }

    UpdateAreasVisibility();

    ProcessCursor(pressedPoint, SEARCH_BACKWARD);
}

bool HUDSystem::OnInput(UIEvent* currentInput)
{
    bool findPivot = selectionContainer.selectedNodes.size() == 1 && IsKeyPressed(KeyboardProxy::KEY_CTRL) && IsKeyPressed(KeyboardProxy::KEY_ALT);
    eSearchOrder searchOrder = findPivot ? SEARCH_BACKWARD : SEARCH_FORWARD;
    switch (currentInput->phase)
    {
    case UIEvent::Phase::MOVE:
        ProcessCursor(currentInput->point, searchOrder);
        return false;
    case UIEvent::Phase::BEGAN:
    {
        ProcessCursor(currentInput->point, searchOrder);
        pressedPoint = currentInput->point;
        if (activeAreaInfo.area != HUDAreaInfo::NO_AREA || currentInput->tid != UIEvent::BUTTON_1)
        {
            return true;
        }
        //check that we can draw rect
        Vector<ControlNode*> nodes;
        Vector<ControlNode*> nodesUnderPoint;
        Vector2 point = currentInput->point;
        auto predicate = [point](const UIControl* control) -> bool {
            return control->GetVisibleForUIEditor() && control->IsPointInside(point);
        };
        systemManager->CollectControlNodes(std::back_inserter(nodesUnderPoint), predicate);
        bool noHudableControls = nodes.empty();
        bool hotKeyDetected = IsKeyPressed(KeyboardProxy::KEY_CTRL);
        SetCanDrawRect(hotKeyDetected || noHudableControls);
        return canDrawRect;
    }
    case UIEvent::Phase::DRAG:
        dragRequested = true;

        if (canDrawRect)
        {
            Vector2 point(pressedPoint);
            Vector2 size(currentInput->point - pressedPoint);
            if (size.x < 0.0f)
            {
                point.x += size.x;
                size.x *= -1.0f;
            }
            if (size.y <= 0.0f)
            {
                point.y += size.y;
                size.y *= -1.0f;
            }
            selectionRectControl->SetRect(Rect(point, size));
            systemManager->SelectionRectChanged.Emit(selectionRectControl->GetAbsoluteRect());
        }
        return true;
    case UIEvent::Phase::ENDED:
    {
        ProcessCursor(currentInput->point, searchOrder);
        selectionRectControl->SetSize(Vector2());
        SetCanDrawRect(false);
        dragRequested = false;
        bool retVal = (pressedPoint - currentInput->point).Length() > 0;
        InvalidatePressedPoint();
        return retVal;
    }
    default:
        return false;
    }
    return false;
}

void HUDSystem::OnRootContolsChanged(const EditorSystemsManager::SortedPackageBaseNodeSet& rootControls)
{
    hudVisible = rootControls.size() == 1;
    UpdateAreasVisibility();
}

void HUDSystem::OnEmulationModeChanged(bool emulationMode)
{
    if (emulationMode)
    {
        systemManager->GetRootControl()->RemoveControl(hudControl.Get());
    }
    else
    {
        systemManager->GetRootControl()->AddControl(hudControl.Get());
    }
}

void HUDSystem::OnMagnetLinesChanged(const Vector<MagnetLineInfo>& magnetLines)
{
    static const float32 axtraSizeValue = 50.0f;
    for (auto& magnetControl : magnetControls)
    {
        hudControl->RemoveControl(magnetControl.Get());
    }
    magnetControls.clear();

    for (const auto& magnetTargetControl : magnetTargetControls)
    {
        hudControl->RemoveControl(magnetTargetControl.Get());
    }
    magnetTargetControls.clear();

    for (const MagnetLineInfo& line : magnetLines)
    {
        const auto& gd = line.gd;

        auto linePos = line.rect.GetPosition();
        auto lineSize = line.rect.GetSize();

        linePos = RotateVector(linePos, gd->angle);
        linePos *= gd->scale;
        lineSize *= gd->scale;
        Vector2 gdPos = gd->position - RotateVector(gd->pivotPoint * gd->scale, gd->angle);

        MagnetLineControl* lineControl = new MagnetLineControl(Rect(linePos + gdPos, lineSize));
        Vector2 extraSize(line.axis == Vector2::AXIS_X ? axtraSizeValue : 0.0f, line.axis == Vector2::AXIS_Y ? axtraSizeValue : 0.0f);
        lineControl->SetSize(lineControl->GetSize() + extraSize);
        lineControl->SetPivotPoint(extraSize / 2.0f);
        lineControl->SetAngle(line.gd->angle);
        hudControl->AddControl(lineControl);
        magnetControls.emplace_back(lineControl);

        linePos = line.targetRect.GetPosition();
        lineSize = line.targetRect.GetSize();

        linePos = RotateVector(linePos, gd->angle);
        linePos *= gd->scale;
        lineSize *= gd->scale;

        MagnetLineControl* rectControl = new MagnetLineControl(Rect(linePos + gdPos, lineSize));
        rectControl->SetAngle(line.gd->angle);
        hudControl->AddControl(rectControl);
        magnetTargetControls.emplace_back(rectControl);
    }
}

void HUDSystem::ProcessCursor(const Vector2& pos, eSearchOrder searchOrder)
{
    SetNewArea(GetControlArea(pos, searchOrder));
}

HUDAreaInfo HUDSystem::GetControlArea(const Vector2& pos, eSearchOrder searchOrder) const
{
    uint32 end = HUDAreaInfo::AREAS_BEGIN;
    int sign = 1;
    if (searchOrder == SEARCH_BACKWARD)
    {
        if (HUDAreaInfo::AREAS_BEGIN != HUDAreaInfo::AREAS_COUNT)
        {
            end = HUDAreaInfo::AREAS_COUNT - 1;
        }
        sign = -1;
    }
    for (uint32 i = HUDAreaInfo::AREAS_BEGIN; i < HUDAreaInfo::AREAS_COUNT; ++i)
    {
        for (const auto& iter : sortedControlList)
        {
            ControlNode* node = dynamic_cast<ControlNode*>(iter);
            DVASSERT(nullptr != node);
            auto findIter = hudMap.find(node);
            DVASSERT_MSG(findIter != hudMap.end(), "hud map corrupted");
            const auto& hud = findIter->second;
            if (hud->container->GetVisible())
            {
                HUDAreaInfo::eArea area = static_cast<HUDAreaInfo::eArea>(end + sign * i);
                auto hudControlsIter = hud->hudControls.find(area);
                if (hudControlsIter != hud->hudControls.end())
                {
                    const auto& controlContainer = hudControlsIter->second;
                    if (controlContainer->GetVisibleForUIEditor() && controlContainer->IsPointInside(pos))
                    {
                        return HUDAreaInfo(hud->node, area);
                    }
                }
            }
        }
    }
    return HUDAreaInfo();
}

void HUDSystem::SetNewArea(const HUDAreaInfo& areaInfo)
{
    if (activeAreaInfo.area != areaInfo.area || activeAreaInfo.owner != areaInfo.owner)
    {
        activeAreaInfo = areaInfo;
        systemManager->ActiveAreaChanged.Emit(activeAreaInfo);
    }
}

void HUDSystem::SetCanDrawRect(bool canDrawRect_)
{
    if (canDrawRect != canDrawRect_)
    {
        canDrawRect = canDrawRect_;
        if (!canDrawRect)
        {
            selectionRectControl->SetSize(Vector2());
        }
    }
}

void HUDSystem::UpdateAreasVisibility()
{
    for (auto& pair : hudMap)
    {
        pair.second->container->SetVisibleInSystems(hudVisible);
    }

    bool showAreas = sortedControlList.size() == 1;

    for (const auto& iter : sortedControlList)
    {
        ControlNode* node = dynamic_cast<ControlNode*>(iter);
        DVASSERT(nullptr != node);
        auto findIter = hudMap.find(node);
        DVASSERT_MSG(findIter != hudMap.end(), "hud map corrupted");
        const auto& hud = findIter->second;
        for (HUDAreaInfo::eArea area : AreasToHide)
        {
            auto hudControlsIter = hud->hudControls.find(area);
            if (hudControlsIter != hud->hudControls.end())
            {
                const auto& controlContainer = hudControlsIter->second;
                controlContainer->SetSystemVisible(showAreas);
            }
        }
    }
}

void HUDSystem::InvalidatePressedPoint()
{
    pressedPoint.Set(std::numeric_limits<float32>::max(), std::numeric_limits<float32>::max());
}
