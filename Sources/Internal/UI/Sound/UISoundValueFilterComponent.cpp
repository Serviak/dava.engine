#include "UISoundValueFilterComponent.h"

#include "UI/UIControl.h"

namespace DAVA
{
UISoundValueFilterComponent::UISoundValueFilterComponent()
{
}

UISoundValueFilterComponent::UISoundValueFilterComponent(const UISoundValueFilterComponent& src)
    : step(src.step)
    , normalizedValue(src.normalizedValue)
{
}

UISoundValueFilterComponent* UISoundValueFilterComponent::Clone() const
{
    return new UISoundValueFilterComponent(*this);
}

float32 UISoundValueFilterComponent::GetStep() const
{
    return step;
}

void UISoundValueFilterComponent::SetStep(float32 step_)
{
    step = step_;
}
}
