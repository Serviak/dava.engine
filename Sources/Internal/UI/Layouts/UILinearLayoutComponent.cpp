#include "UILinearLayoutComponent.h"

namespace DAVA
{
UILinearLayoutComponent::UILinearLayoutComponent()
{
    
}

UILinearLayoutComponent::UILinearLayoutComponent(const UILinearLayoutComponent &src)
    : flags(src.flags)
    , padding(src.padding)
    , spacing(src.spacing)
{
    
}

UILinearLayoutComponent::~UILinearLayoutComponent()
{
    
}

UILinearLayoutComponent* UILinearLayoutComponent::Clone() const
{
    return new UILinearLayoutComponent(*this);
}

bool UILinearLayoutComponent::IsEnabled() const
{
    return flags.test(FLAG_ENABLED);
}

void UILinearLayoutComponent::SetEnabled(bool enabled)
{
    flags.set(FLAG_ENABLED, enabled);
}

UILinearLayoutComponent::eOrientation UILinearLayoutComponent::GetOrientation() const
{
    return flags.test(FLAG_ORIENTATION_VERTICAL) ? VERTICAL : HORIZONTAL;
}

void UILinearLayoutComponent::SetOrientation(eOrientation newOrientation)
{
    flags.set(FLAG_ORIENTATION_VERTICAL, newOrientation == VERTICAL);
}

int32 UILinearLayoutComponent::GetOrientationAsInt() const
{
    return static_cast<int32>(GetOrientation());
}

void UILinearLayoutComponent::SetOrientationFromInt(int32 orientation)
{
    SetOrientation(static_cast<eOrientation>(orientation));
}


float32 UILinearLayoutComponent::GetPadding() const
{
    return padding;
}

void UILinearLayoutComponent::SetPadding(float32 newPadding)
{
    padding = newPadding;
}

float32 UILinearLayoutComponent::GetSpacing() const
{
    return spacing;
}

void UILinearLayoutComponent::SetSpacing(float32 newSpacing)
{
    spacing = newSpacing;
}

bool UILinearLayoutComponent::IsDynamicPadding() const
{
    return flags.test(FLAG_DYNAMIC_PADDING);
}

void UILinearLayoutComponent::SetDynamicPadding(bool dynamic)
{
    flags.set(FLAG_DYNAMIC_PADDING, dynamic);
}

bool UILinearLayoutComponent::IsDynamicSpacing() const
{
    return flags.test(FLAG_DYNAMIC_SPACING);
}

void UILinearLayoutComponent::SetDynamicSpacing(bool dynamic)
{
    flags.set(FLAG_DYNAMIC_SPACING);
}

bool UILinearLayoutComponent::IsSkipInvisibleControls() const
{
    return flags.test(FLAG_SKIP_INVISIBLE_CONTROLS);
}

void UILinearLayoutComponent::SetSkipInvisibleControls(bool skip)
{
    flags.set(FLAG_SKIP_INVISIBLE_CONTROLS, skip);
}

bool UILinearLayoutComponent::IsUseRtl() const
{
    return flags.test(FLAG_RTL);
}

void UILinearLayoutComponent::SetUseRtl(bool use)
{
    flags.set(FLAG_RTL, use);
}

}
