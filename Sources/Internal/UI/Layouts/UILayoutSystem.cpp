#include "UILayoutSystem.h"

#include "UILinearLayoutComponent.h"
#include "UIFlowLayoutComponent.h"
#include "UIAnchorComponent.h"
#include "UISizePolicyComponent.h"

#include "SizeMeasuringAlgorithm.h"
#include "LinearLayoutAlgorithm.h"
#include "FlowLayoutAlgorithm.h"
#include "AnchorLayoutAlgorithm.h"

#include "UI/UIControl.h"
#include "UI/Styles/UIStyleSheetPropertyDataBase.h"

#include "Concurrency/Thread.h"

namespace DAVA
{
UILayoutSystem::UILayoutSystem()
{
}

UILayoutSystem::~UILayoutSystem()
{
}

bool UILayoutSystem::IsRtl() const
{
    return isRtl;
}

void UILayoutSystem::SetRtl(bool rtl)
{
    isRtl = rtl;
}

bool UILayoutSystem::IsAutoupdatesEnabled() const
{
    return autoupdatesEnabled;
}

void UILayoutSystem::SetAutoupdatesEnabled(bool enabled)
{
    autoupdatesEnabled = enabled;
}

void UILayoutSystem::ApplyLayout(UIControl* control, bool considerDenendenceOnChildren)
{
    DVASSERT(Thread::IsMainThread() || autoupdatesEnabled == false);

    UIControl* container = control;
    if (considerDenendenceOnChildren)
    {
        container = FindNotDependentOnChildrenControl(container);
    }

    CollectControls(container, true);

    ProcessAxis(Vector2::AXIS_X);
    ProcessAxis(Vector2::AXIS_Y);

    ApplySizesAndPositions();

    layoutData.clear();
}

void UILayoutSystem::ApplyLayoutNonRecursive(UIControl* control)
{
    DVASSERT(Thread::IsMainThread() || autoupdatesEnabled == false);

    CollectControls(control, false);

    ProcessAxis(Vector2::AXIS_X);
    ProcessAxis(Vector2::AXIS_Y);

    ApplyPositions();

    layoutData.clear();
}

void UILayoutSystem::Update(UIControl* root)
{
    if (!(needUpdate || dirty) || !root)
        return;
    UpdateControl(root);
}

UIControl* UILayoutSystem::FindNotDependentOnChildrenControl(UIControl* control) const
{
    UIControl* result = control;
    while (result->GetParent() != nullptr)
    {
        UISizePolicyComponent* sizePolicy = result->GetParent()->GetComponent<UISizePolicyComponent>();
        if (sizePolicy != nullptr && (sizePolicy->IsDependsOnChildren(Vector2::AXIS_X) || sizePolicy->IsDependsOnChildren(Vector2::AXIS_Y)))
        {
            result = result->GetParent();
        }
        else
        {
            break;
        }
    }

    if (result->GetParent())
    {
        result = result->GetParent();
    }

    return result;
}

void UILayoutSystem::CollectControls(UIControl* control, bool recursive)
{
    layoutData.clear();
    layoutData.emplace_back(ControlLayoutData(control));
    CollectControlChildren(control, 0, recursive);
}

void UILayoutSystem::CollectControlChildren(UIControl* control, int32 parentIndex, bool recursive)
{
    int32 index = static_cast<int32>(layoutData.size());
    const List<UIControl*>& children = control->GetChildren();

    layoutData[parentIndex].SetFirstChildIndex(index);
    layoutData[parentIndex].SetLastChildIndex(index + static_cast<int32>(children.size() - 1));

    for (UIControl* child : children)
    {
        layoutData.emplace_back(ControlLayoutData(child));
    }

    if (recursive)
    {
        for (UIControl* child : children)
        {
            CollectControlChildren(child, index, recursive);
            index++;
        }
    }
}

void UILayoutSystem::ProcessAxis(Vector2::eAxis axis)
{
    DoMeasurePhase(axis);
    DoLayoutPhase(axis);
}

void UILayoutSystem::DoMeasurePhase(Vector2::eAxis axis)
{
    int32 lastIndex = static_cast<int32>(layoutData.size() - 1);
    for (int32 index = lastIndex; index >= 0; index--)
    {
        SizeMeasuringAlgorithm(layoutData).Apply(layoutData[index], axis);
    }
}

void UILayoutSystem::DoLayoutPhase(Vector2::eAxis axis)
{
    for (auto it = layoutData.begin(); it != layoutData.end(); ++it)
    {
        UIFlowLayoutComponent* flowLayoutComponent = it->GetControl()->GetComponent<UIFlowLayoutComponent>();
        if (flowLayoutComponent && flowLayoutComponent->IsEnabled())
        {
            FlowLayoutAlgorithm(layoutData, isRtl).Apply(*it, axis);
        }
        else
        {
            UILinearLayoutComponent* linearLayoutComponent = it->GetControl()->GetComponent<UILinearLayoutComponent>();
            if (linearLayoutComponent != nullptr && linearLayoutComponent->IsEnabled() && linearLayoutComponent->GetAxis() == axis)
            {
                LinearLayoutAlgorithm alg(layoutData, isRtl);

                bool inverse = linearLayoutComponent->IsInverse();
                if (isRtl && linearLayoutComponent->IsUseRtl() && linearLayoutComponent->GetAxis() == Vector2::AXIS_X)
                {
                    inverse = !inverse;
                }
                alg.SetInverse(inverse);
                alg.SetSkipInvisible(linearLayoutComponent->IsSkipInvisibleControls());

                alg.SetPadding(linearLayoutComponent->GetPadding());
                alg.SetSpacing(linearLayoutComponent->GetSpacing());

                alg.SetDynamicPadding(linearLayoutComponent->IsDynamicPadding());
                alg.SetDynamicSpacing(linearLayoutComponent->IsDynamicSpacing());

                alg.Apply(*it, axis);
            }
            else
            {
                AnchorLayoutAlgorithm(layoutData, isRtl).Apply(*it, axis, false);
            }
        }
    }
}

void UILayoutSystem::ApplySizesAndPositions()
{
    for (ControlLayoutData& data : layoutData)
    {
        data.ApplyLayoutToControl();
    }
}

void UILayoutSystem::ApplyPositions()
{
    for (ControlLayoutData& data : layoutData)
    {
        data.ApplyOnlyPositionLayoutToControl();
    }
}
void UILayoutSystem::UpdateControl(UIControl* control)
{
    if (!control->IsVisible())
    {
        return;
    }

    if (control->IsLayoutDirty())
    {
        if (IsAutoupdatesEnabled())
        {
            ApplyLayout(control, true);
        }
    }
    else if (control->IsLayoutPositionDirty())
    {
        if (IsAutoupdatesEnabled() && control->GetParent() != nullptr)
        {
            ApplyLayoutNonRecursive(control->GetParent());
        }
    }

    // TODO: For now game has many places where changes in layouts can
    // change hierarchy of controls. In future client want fix this places,
    // after that this code should be replaced by simple for-each.
    const List<UIControl*>& children = control->GetChildren();
    auto it = children.begin();
    auto endIt = children.end();
    while (it != endIt)
    {
        control->isIteratorCorrupted = false;
        UpdateControl(*it);
        if (control->isIteratorCorrupted)
        {
            it = children.begin();
            continue;
        }
        ++it;
    }
}
}
