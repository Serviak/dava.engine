#pragma once

#include <Base/BaseTypes.h>
#include <Base/IntrospectionBase.h>
#include <Functional/Functional.h>
#include <FileSystem/VariantType.h>
#include <UI/Components/UIComponent.h>

class ControlInformation
{
public:
    virtual ~ControlInformation() = default;

    virtual DAVA::FastName GetName() const = 0;
    virtual DAVA::FastName GetPrototype() const = 0;
    virtual DAVA::String GetPrototypePackagePath() const = 0;

    virtual bool HasComponent(DAVA::UIComponent::eType componentType) const = 0;

    virtual void VisitParent(const DAVA::Function<void(const ControlInformation*)>& visitor) const = 0;
    virtual void VisitChildren(const DAVA::Function<void(const ControlInformation*)>& visitor) const = 0;

    virtual DAVA::VariantType GetControlPropertyValue(const DAVA::InspMember* member) const = 0;
};
