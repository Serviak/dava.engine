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

#ifndef __QUICKED_TRANSFORM_SYSTEM_H__
#define __QUICKED_TRANSFORM_SYSTEM_H__

#include "EditorSystems/BaseEditorSystem.h"
#include "EditorSystems/EditorSystemsManager.h"
#include "Base/BaseTypes.h"
#include "Math/Vector.h"
#include "UI/UIControl.h"

namespace DAVA
{
class UIGeometricData;
class UIControl;
}
class TransformSystem final : public BaseEditorSystem
{
public:
    explicit TransformSystem(EditorSystemsManager* parent);
    ~TransformSystem() = default;

    bool OnInput(DAVA::UIEvent* currentInput) override;

private:
    using MoveInfo = std::tuple<ControlNode* /*node*/, AbstractProperty* /*positionProperty*/, const DAVA::UIGeometricData* /*parent gd*/>;
    enum
    {
        NODE_INDEX,
        PROPERTY_INDEX,
        GD_INDEX
    };
    using Directions = DAVA::Array<int, DAVA::Vector2::AXIS_COUNT>;
    using CornersDirections = DAVA::Array<Directions, HUDAreaInfo::CORNERS_COUNT>;
    static const CornersDirections cornersDirections;
    using PropertyDelta = std::pair<AbstractProperty* /*property*/, DAVA::VariantType /*delta*/>;

    void OnSelectionChanged(const SelectedNodes& selected, const SelectedNodes& deselected);
    void OnActiveAreaChanged(const HUDAreaInfo& areaInfo);

    bool ProcessKey(const DAVA::int32 key);

    bool ProcessDrag(DAVA::Vector2 point);
    void ResizeControl(DAVA::Vector2 delta, bool withPivot, bool rateably);
    DAVA::Vector2 AdjustToMinimumSize(DAVA::Vector2 delta);
    DAVA::Vector2 AdjustResize(DAVA::Vector2 deltaSize, DAVA::Vector2 deltaPosition, DAVA::Vector2 transfromPoint, Directions directions);
    void MovePivot(DAVA::Vector2 delta);
    void EmitMagnetLinesForPivot(DAVA::Vector2& target);
    DAVA::Vector2 AdjustPivot(DAVA::Vector2& delta);
    void Rotate(DAVA::Vector2 pos);
    bool AdjustRotate(DAVA::float32 deltaAngle, DAVA::float32 originalAngle, DAVA::float32& finalAngle);
    void MoveAllSelectedControls(DAVA::Vector2 delta, bool canAdjust);
    DAVA::Vector2 AdjustMove(DAVA::Vector2 delta, DAVA::Vector<MagnetLineInfo>& magnetLines, const DAVA::UIGeometricData* parentGD, const DAVA::UIControl* control);

    void CorrectNodesToMove();
    void UpdateNeighboursToMove();

    HUDAreaInfo::eArea activeArea = HUDAreaInfo::NO_AREA;
    ControlNode* activeControlNode = nullptr;
    DAVA::Vector2 prevPos;
    DAVA::Vector2 extraDelta;
    SelectedControls selectedControlNodes;
    DAVA::List<MoveInfo> nodesToMove;
    DAVA::Vector<DAVA::UIControl*> neighbours;
    size_t currentHash = 0;

    DAVA::UIGeometricData parentGeometricData;
    DAVA::UIGeometricData controlGeometricData;
    AbstractProperty* sizeProperty = nullptr;
    AbstractProperty* positionProperty = nullptr;
    AbstractProperty* angleProperty = nullptr;
    AbstractProperty* pivotProperty = nullptr;
};

#endif // __QUICKED_TRANSFORM_SYSTEM_H__
