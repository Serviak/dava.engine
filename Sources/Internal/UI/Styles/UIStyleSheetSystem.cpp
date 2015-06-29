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

#include "UI/Styles/UIStyleSheetSystem.h"
#include "UI/Styles/UIStyleSheet.h"
#include "UI/UIControl.h"
#include "UI/UIControlPackageContext.h"
#include "UI/Components/UIComponent.h"
#include "Platform/SystemTimer.h"
#include "Animation/LinearPropertyAnimation.h"
#include "Animation/AnimationManager.h"

namespace DAVA
{

namespace
{
    const int32 PROPERTY_ANIMATION_GROUP_OFFSET = 100000;
}

struct ImmediatePropertySetter
{
    void operator ()(UIControl* control, void* targetObject, const InspMember* targetIntrospectionMember) const
    {
        control->StopAnimations(PROPERTY_ANIMATION_GROUP_OFFSET + propertyIndex);
        targetIntrospectionMember->SetValue(targetObject, value);
    }

    uint32 propertyIndex;
    const VariantType& value;
};

struct AnimatedPropertySetter
{
    template<typename T>
    void Animate(UIControl* control, void* targetObject, const InspMember* targetIntrospectionMember, const T& startValue, const T& endValue) const
    {
        const int32 track = PROPERTY_ANIMATION_GROUP_OFFSET + propertyIndex;
        LinearPropertyAnimation<T>* currentAnimation = DynamicTypeCheck<LinearPropertyAnimation<T>*>(AnimationManager::Instance()->FindPlayingAnimation(control, track));

        if (!currentAnimation || currentAnimation->GetEndValue() != endValue)
        {
            if (currentAnimation)
                control->StopAnimations(track);

            if (targetIntrospectionMember->Value(targetObject) != value)
            {
                (new LinearPropertyAnimation<T>(control, targetObject, targetIntrospectionMember, startValue, endValue, time, transitionFunction))->Start(track);
            }
        }
    }

    void operator ()(UIControl* control, void* targetObject, const InspMember* targetIntrospectionMember) const
    {
        switch (value.GetType())
        {
        case VariantType::TYPE_VECTOR2:
            Animate<Vector2>(control, targetObject, targetIntrospectionMember, targetIntrospectionMember->Value(targetObject).AsVector2(), value.AsVector2());
            break;
        case VariantType::TYPE_VECTOR3:
            Animate<Vector3>(control, targetObject, targetIntrospectionMember, targetIntrospectionMember->Value(targetObject).AsVector3(), value.AsVector3());
            break;
        case VariantType::TYPE_VECTOR4:
            Animate<Vector4>(control, targetObject, targetIntrospectionMember, targetIntrospectionMember->Value(targetObject).AsVector4(), value.AsVector4());
            break;
        case VariantType::TYPE_FLOAT:
            Animate<float32>(control, targetObject, targetIntrospectionMember, targetIntrospectionMember->Value(targetObject).AsFloat(), value.AsFloat());
            break;
        case VariantType::TYPE_COLOR:
            Animate<Color>(control, targetObject, targetIntrospectionMember, targetIntrospectionMember->Value(targetObject).AsColor(), value.AsColor());
            break;
        default:
            DVASSERT_MSG(false, "Non-animatable property");
        }
    }

    uint32 propertyIndex;
    const VariantType& value;
    Interpolation::FuncType transitionFunction;
    float32 time;
};

UIStyleSheetSystem::UIStyleSheetSystem()
{

}

UIStyleSheetSystem::~UIStyleSheetSystem()
{

}

void UIStyleSheetSystem::ProcessControl(UIControl* control)
{
    UIControlPackageContext* packageContext = control->GetPackageContext();
    const UIStyleSheetPropertyDataBase* propertyDB = UIStyleSheetPropertyDataBase::Instance();

    if (packageContext)
    {
        UIStyleSheetPropertySet appliedProperties;
        const UIStyleSheetPropertySet& localControlProperties = control->GetLocalPropertySet();
        const auto& styleSheets = packageContext->GetSortedStyleSheets();
        for (const UIStyleSheet* styleSheet : styleSheets)
        {
            if (StyleSheetMatchesControl(styleSheet, control))
            {
                const auto& propertyTable = styleSheet->GetPropertyTable()->GetProperties();
                for (const auto& iter : propertyTable)
                {
                    if (!appliedProperties.test(iter.propertyIndex) && !localControlProperties.test(iter.propertyIndex))
                    {
                        appliedProperties.set(iter.propertyIndex);

                        if (iter.transition && control->GetStyleSheetInitialized())
                            DoForAllPropertyInstances(control, iter.propertyIndex, AnimatedPropertySetter{ iter.propertyIndex, iter.value, iter.transitionFunction, iter.transitionTime });
                        else
                            DoForAllPropertyInstances(control, iter.propertyIndex, ImmediatePropertySetter{ iter.propertyIndex, iter.value });
                    }
                }
            }
        }

        const UIStyleSheetPropertySet& propertiesToReset = control->GetStyledPropertySet() & ~appliedProperties;
        if (propertiesToReset.any())
        {
            for (uint32 propertyIndex = 0; propertyIndex < UIStyleSheetPropertyDataBase::STYLE_SHEET_PROPERTY_COUNT; ++propertyIndex)
            {
                if (propertiesToReset.test(propertyIndex))
                {
                    const UIStyleSheetPropertyDescriptor& propertyDescr = propertyDB->GetStyleSheetPropertyByIndex(propertyIndex);
                    DoForAllPropertyInstances(control, propertyIndex, ImmediatePropertySetter{ propertyIndex, propertyDescr.defaultValue });
                }
            }
        }

        control->SetStyledPropertySet(appliedProperties);
    }

    control->MarkStyleSheetAsUpdated();

    for (UIControl* child : control->GetChildren())
    {
        ProcessControl(child);
    }
}

bool UIStyleSheetSystem::StyleSheetMatchesControl(const UIStyleSheet* styleSheet, UIControl* control)
{
    UIControl* currentControl = control;

    auto endIter = styleSheet->GetSelectorChain().rend();
    for (auto selectorIter = styleSheet->GetSelectorChain().rbegin(); selectorIter != endIter; ++selectorIter)
    {
        if (!currentControl || !SelectorMatchesControl(*selectorIter, currentControl))
            return false;

        currentControl = currentControl->GetParent();
    }

    return true;
}

bool UIStyleSheetSystem::SelectorMatchesControl(const UIStyleSheetSelector& selector, UIControl* control)
{
    if (((selector.controlStateMask & control->GetState()) != selector.controlStateMask)
        || (selector.name.IsValid() && selector.name != control->GetFastName())
        || (!selector.controlClassName.empty() && selector.controlClassName != control->GetClassName()))
        return false;

    for (const FastName& clazz : selector.classes)
    {
        if (!control->HasClass(clazz))
            return false;
    }

    return true;
}

template <typename CallbackType>
void UIStyleSheetSystem::DoForAllPropertyInstances(UIControl* control, uint32 propertyIndex, const CallbackType& action)
{
    const UIStyleSheetPropertyDataBase* propertyDB = UIStyleSheetPropertyDataBase::Instance();

    const UIStyleSheetPropertyDescriptor& descr = propertyDB->GetStyleSheetPropertyByIndex(propertyIndex);

    for (const UIStyleSheetPropertyTargetMember& targetMember : descr.targetMembers)
    {
        switch (targetMember.propertyOwner)
        {
        case ePropertyOwner::CONTROL:
        {
            const InspInfo* typeInfo = control->GetTypeInfo();
            do
            {
                if (typeInfo == targetMember.typeInfo)
                {
                    action(control, control, targetMember.memberInfo);
                    break;
                }
                typeInfo = typeInfo->BaseInfo();
            } while (typeInfo);

            break;
        }
        case ePropertyOwner::BACKGROUND:
            if (control->GetBackgroundComponentsCount() > 0)
                action(control, control->GetBackgroundComponent(0), targetMember.memberInfo);
            break;
        case ePropertyOwner::COMPONENT:
            if (UIComponent* component = control->GetComponent(targetMember.componentType))
                action(control, component, targetMember.memberInfo);
            break;
        default:
            DVASSERT(false);
            break;
        }
    }
}

}
