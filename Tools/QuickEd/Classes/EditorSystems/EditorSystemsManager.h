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

#ifndef __QUICKED_SYSTEMS_MANAGER_H__
#define __QUICKED_SYSTEMS_MANAGER_H__

#include "Base/BaseTypes.h"
#include "Base/RefPtr.h"
#include "Functional/Signal.h"
#include "EditorSystems/SelectionContainer.h"
#include "Math/Rect.h"
#include "Math/Vector.h"
#include "Model/PackageHierarchy/PackageListener.h"

namespace DAVA
{
class UIControl;
class UIEvent;
class VariantType;
class UIGeometricData;
}

struct HUDAreaInfo
{
    enum eArea
    {
        AREAS_BEGIN,
        ROTATE_AREA = AREAS_BEGIN,
        TOP_LEFT_AREA,
        TOP_CENTER_AREA,
        TOP_RIGHT_AREA,
        CENTER_LEFT_AREA,
        CENTER_RIGHT_AREA,
        BOTTOM_LEFT_AREA,
        BOTTOM_CENTER_AREA,
        BOTTOM_RIGHT_AREA,
        PIVOT_POINT_AREA,
        FRAME_AREA,
        NO_AREA,
        CORNERS_BEGIN = TOP_LEFT_AREA,
        CORNERS_COUNT = PIVOT_POINT_AREA - TOP_LEFT_AREA + CORNERS_BEGIN,
        AREAS_COUNT = NO_AREA - AREAS_BEGIN
    };
    HUDAreaInfo(ControlNode* owner_ = nullptr, eArea area_ = NO_AREA)
        : owner(owner_)
        , area(area_)
    {
        DVASSERT((owner != nullptr && area != HUDAreaInfo::NO_AREA) || (owner == nullptr && area == HUDAreaInfo::NO_AREA));
    }
    ControlNode* owner = nullptr;
    eArea area = NO_AREA;
};

struct MagnetLineInfo
{
    MagnetLineInfo(const DAVA::Rect& rect_, const DAVA::UIGeometricData* gd_, DAVA::Vector2::eAxis axis_)
        : absoluteRect(rect_)
        , gd(gd_)
        , axis(axis_)
    {
    }
    DAVA::Rect absoluteRect;
    const DAVA::UIGeometricData* gd;
    const DAVA::Vector2::eAxis axis;
};

class BaseEditorSystem;
class AbstractProperty;
class PackageNode;

bool CompareByLCA(PackageBaseNode* left, PackageBaseNode* right);

class EditorSystemsManager : PackageListener
{
public:
    using SortedPackageBaseNodeSet = DAVA::Set<PackageBaseNode*, std::function<bool(PackageBaseNode*, PackageBaseNode*)>>;

    explicit EditorSystemsManager(PackageNode* package);
    ~EditorSystemsManager();

    PackageNode* GetPackage();

    DAVA::UIControl* GetRootControl();
    DAVA::UIControl* GetScalableControl();

    void Deactivate();
    void Activate();

    bool OnInput(DAVA::UIEvent* currentInput);

    void SetEmulationMode(bool emulationMode);

    void CollectControlNodesByPos(DAVA::Vector<ControlNode*>& controlNodes, const DAVA::Vector2& pos) const;
    void CollectControlNodesByRect(SelectedControls& controlNodes, const DAVA::Rect& rect) const;

    DAVA::Signal<const SelectedNodes& /*selected*/, const SelectedNodes& /*deselected*/> SelectionChanged;
    DAVA::Signal<const HUDAreaInfo& /*areaInfo*/> ActiveAreaChanged;
    DAVA::Signal<const DAVA::Rect& /*selectionRectControl*/> SelectionRectChanged;
    DAVA::Signal<bool> EmulationModeChangedSignal;
    DAVA::Signal<> CanvasSizeChanged;
    DAVA::Signal<DAVA::Vector2> rootControlPositionChanged;
    DAVA::Signal<const DAVA::Vector<std::tuple<ControlNode*, AbstractProperty*, DAVA::VariantType>>& /*properties*/, size_t /*hash*/> PropertiesChanged;
    DAVA::Signal<const DAVA::Vector<ControlNode*>& /*nodes*/, const DAVA::Vector2& /*pos*/, ControlNode*& /*selectedNode*/> SelectionByMenuRequested;
    DAVA::Signal<const SortedPackageBaseNodeSet&> EditingRootControlsChanged;
    DAVA::Signal<const DAVA::Vector<MagnetLineInfo>& /*magnetLines*/> MagnetLinesChanged;
    DAVA::Signal<DAVA::Vector2 /*new position*/> RootControlPositionChanged;

private:
    class RootControl;
    void OnSelectionChanged(const SelectedNodes& selected, const SelectedNodes& deselected);
    void CollectControlNodesByPosImpl(DAVA::Vector<ControlNode*>& controlNodes, const DAVA::Vector2& pos, ControlNode* node) const;
    void CollectControlNodesByRectImpl(SelectedControls& controlNodes, const DAVA::Rect& rect, ControlNode* node) const;

    void ControlWasRemoved(ControlNode* node, ControlsContainerNode* from) override;
    void ControlWasAdded(ControlNode* node, ControlsContainerNode* /*destination*/, int /*index*/) override;
    void SetPreviewMode(bool mode);
    DAVA::RefPtr<RootControl> rootControl;
    DAVA::RefPtr<DAVA::UIControl> scalableControl;

    DAVA::List<std::unique_ptr<BaseEditorSystem>> systems;

    PackageNode* package = nullptr;
    SelectedControls selectedControlNodes;
    SortedPackageBaseNodeSet editingRootControls;
    bool previewMode = true;
};

#endif // __QUICKED_SYSTEMS_MANAGER_H__
