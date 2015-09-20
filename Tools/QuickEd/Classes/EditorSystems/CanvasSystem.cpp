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

#include "CanvasSystem.h"
#include "EditorSystems/EditorSystemsManager.h"
#include "UI/UIControl.h"
#include "Base/BaseTypes.h"
#include "Model/PackageHierarchy/PackageBaseNode.h"
#include "Model/PackageHierarchy/ControlNode.h"
#include "Model/PackageHierarchy/PackageNode.h"
#include "Model/PackageHierarchy/PackageControlsNode.h"
#include "Render/2D/Systems/RenderSystem2D.h"
#include "Render/Shader.h"
#include "Model/ControlProperties/PropertyListener.h"
#include "Model/ControlProperties/RootProperty.h"

using namespace DAVA;

class GridCanvas : public UIControl, private PropertyListener
{
public:
    GridCanvas(CanvasSystem* canvasSystem, RootProperty* property);
    ~GridCanvas() override;
    void Init(UIControl* control);
    void PropertyChanged(AbstractProperty* property) override;
    UIControl* GetPositionHolder();

private:
    void UpdateSprite();
    void Draw(const UIGeometricData& geometricData) override;
    RootProperty* rootProperty = nullptr;
    ScopedPtr<UIControl> positionHolder;
    CanvasSystem* canvasSystem = nullptr;
    UIControl* nestedControl = nullptr;
};

GridCanvas::GridCanvas(CanvasSystem* canvasSystem_, RootProperty* property)
    : UIControl()
    , rootProperty(property)
    , positionHolder(new UIControl())
    , canvasSystem(canvasSystem_)
{
    property->AddListener(this);
}

GridCanvas::~GridCanvas()
{
    rootProperty->RemoveListener(this);
}

void GridCanvas::Init(UIControl* control)
{
    nestedControl = control;
    SetName("GridCanvas");
    positionHolder->SetName("Position holder");
    AddControl(positionHolder);

    background->SetSprite("~res:/Gfx/CheckeredBg", 0);
    background->SetDrawType(UIControlBackground::DRAW_TILED);
    background->SetShader(RenderSystem2D::TEXTURE_MUL_FLAT_COLOR);

    SetSize(control->GetSize());
    positionHolder->SetSize(control->GetSize() * control->GetScale());
    positionHolder->SetPosition(-control->GetPosition());
    positionHolder->SetPivot(-control->GetPivot());
    positionHolder->SetAngle(-control->GetAngle());
    UpdateSprite();
}

void GridCanvas::PropertyChanged(AbstractProperty* property)
{
    const String& name = property->GetName();
    if (name == "Size" || name == "Scale")
    {
        Vector2 size = nestedControl->GetSize();
        size *= nestedControl->GetScale();
        SetSize(size);
        positionHolder->SetSize(size);
        canvasSystem->LayoutCanvas();
    }
    else if (name == "Position")
    {
        Vector2 position = property->GetValue().AsVector2();
        positionHolder->SetPosition(-position);
        UpdateSprite();
    }
    else if (name == "Pivot")
    {
        Vector2 pivot = property->GetValue().AsVector2();
        positionHolder->SetPivot(-pivot);
        UpdateSprite();
    }
    else if (name == "Angle")
    {
        float32 angle = property->GetValue().AsFloat();
        positionHolder->SetAngle(-DegToRad(angle));
        UpdateSprite();
    }
}

UIControl* GridCanvas::GetPositionHolder()
{
    return positionHolder;
}

void GridCanvas::UpdateSprite()
{
    bool transformed = !positionHolder->GetPosition().IsZero() || !positionHolder->GetPivot().IsZero() || positionHolder->GetAngle() != 0;
    if (transformed)
    {
        background->SetSprite("~res:/Gfx/CheckeredBg2", 0);
    }
    else
    {
        background->SetSprite("~res:/Gfx/CheckeredBg", 0);
    }
}

void GridCanvas::Draw(const UIGeometricData& geometricData)
{
    if (0.0f != geometricData.scale.x)
    {
        float32 invScale = 1.0f / geometricData.scale.x;
        UIGeometricData unscaledGd;
        unscaledGd.scale = Vector2(invScale, invScale);
        unscaledGd.size = geometricData.size * geometricData.scale.x;
        unscaledGd.AddGeometricData(geometricData);
        GetBackground()->Draw(unscaledGd);
    }
}

CanvasSystem::CanvasSystem(EditorSystemsManager* parent)
    : BaseEditorSystem(parent)
    , controlsCanvas(new UIControl())
{
    controlsCanvas->SetName("controls canvas");
    systemManager->GetPackage()->AddListener(this);

    auto controlsNode = systemManager->GetPackage()->GetPackageControlsNode();
    for (int index = 0; index < controlsNode->GetCount(); ++index)
    {
        AddRootControl(controlsNode->Get(index));
    }
}

void CanvasSystem::OnActivated()
{
    systemManager->GetScalableControl()->AddControl(controlsCanvas);

    LayoutCanvas();
}

void CanvasSystem::OnDeactivated()
{
    controlsCanvas->RemoveFromParent();
}

void CanvasSystem::ControlWillBeRemoved(::ControlNode* node, ControlsContainerNode* from)
{
    if (nullptr == node->GetParent())
    {
        UIControl* removedControl = node->GetControl()->GetParent();
        controlsCanvas->RemoveControl(removedControl);
        LayoutCanvas();
    }
}

void CanvasSystem::ControlWasAdded(ControlNode* node, ControlsContainerNode* /*destination*/, int /*index*/)
{
    auto packageControlsNode = systemManager->GetPackage()->GetPackageControlsNode();
    if (packageControlsNode == node->GetParent())
    {
        AddRootControl(node);
        LayoutCanvas();
    }
}

void CanvasSystem::SetRootControls(const Vector<ControlNode*>& controls)
{
    for (auto node : controls)
    {
        AddRootControl(node);
    }
}

void CanvasSystem::AddRootControl(ControlNode* controlNode)
{
    ScopedPtr<GridCanvas> gridControl(new GridCanvas(this, controlNode->GetRootProperty()));
    UIControl* control = controlNode->GetControl();
    gridControl->Init(control);
    const Vector2& size = control->GetSize();
    gridControl->SetSize(size);
    gridControl->GetPositionHolder()->SetSize(size);
    gridControl->GetPositionHolder()->AddControl(control);
    controlsCanvas->AddControl(gridControl);
}

void CanvasSystem::LayoutCanvas()
{
    float32 maxWidth = 0.0f;
    float32 totalHeight = 0.0f;
    const int spacing = 5;
    int childrenCount = controlsCanvas->GetChildren().size();
    if (childrenCount > 1)
    {
        totalHeight += spacing * (childrenCount - 1);
    }
    for (auto control : controlsCanvas->GetChildren())
    {
        maxWidth = Max(maxWidth, control->GetSize().x);
        totalHeight += control->GetSize().y;
    }

    float32 curY = 0.0f;
    for (auto control : controlsCanvas->GetChildren())
    {
        Rect rect = control->GetRect();
        rect.y = curY;
        rect.x = (maxWidth - rect.dx) / 2.0f;
        control->SetRect(rect);
        curY += rect.dy + spacing;
    }
    Vector2 size(maxWidth, totalHeight);
    controlsCanvas->SetSize(size);
    systemManager->GetRootControl()->SetSize(size);
    systemManager->CanvasSizeChanged.Emit();
}
