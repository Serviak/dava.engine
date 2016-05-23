#include "InternalControlPropertiesSection.h"

#include "PropertyVisitor.h"
#include "IntrospectionProperty.h"

#include "UI/UIControl.h"

using namespace DAVA;

InternalControlPropertiesSection::InternalControlPropertiesSection(DAVA::UIControl* aControl, int num, const InternalControlPropertiesSection* sourceSection, eCloneType cloneType)
    : SectionProperty("")
    , control(SafeRetain(aControl))
    , internalControl(nullptr)
    , internalControlNum(num)
{
    name = control->GetInternalControlName(internalControlNum) + control->GetInternalControlDescriptions();

    internalControl = SafeRetain(control->GetInternalControl(num));
    if (internalControl == nullptr && sourceSection != nullptr && sourceSection->GetInternalControl() != nullptr)
    {
        internalControl = control->CreateInternalControl(num);
        control->SetInternalControl(num, internalControl);
    }

    if (internalControl)
    {
        const InspInfo* insp = internalControl->GetTypeInfo();
        for (int j = 0; j < insp->MembersCount(); j++)
        {
            const InspMember* member = insp->Member(j);

            IntrospectionProperty* sourceProperty = nullptr == sourceSection ? nullptr : sourceSection->FindProperty(member);
            IntrospectionProperty* prop = IntrospectionProperty::Create(internalControl, member, sourceProperty, cloneType);
            AddProperty(prop);
            SafeRelease(prop);
        }
    }
}

InternalControlPropertiesSection::~InternalControlPropertiesSection()
{
    SafeRelease(internalControl);
    SafeRelease(control);
}

UIControl* InternalControlPropertiesSection::GetInternalControl() const
{
    return internalControl;
}

void InternalControlPropertiesSection::CreateInternalControl()
{
    if (!internalControl)
    {
        internalControl = control->CreateInternalControl(internalControlNum);
        control->SetInternalControl(internalControlNum, internalControl);

        const InspInfo* insp = internalControl->GetTypeInfo();
        for (int j = 0; j < insp->MembersCount(); j++)
        {
            const InspMember* member = insp->Member(j);

            IntrospectionProperty* prop = IntrospectionProperty::Create(internalControl, member, nullptr, CT_COPY);
            AddProperty(prop);
            SafeRelease(prop);
        }
    }
}

bool InternalControlPropertiesSection::HasChanges() const
{
    return internalControl && SectionProperty::HasChanges();
}

void InternalControlPropertiesSection::Accept(PropertyVisitor* visitor)
{
    visitor->VisitInternalControlSection(this);
}
