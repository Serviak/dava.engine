#include "ComponentPropertiesSection.h"

#include "IntrospectionProperty.h"
#include "PropertyVisitor.h"

#include "UI/UIControl.h"

#include "Utils/StringFormat.h"

using namespace DAVA;

ComponentPropertiesSection::ComponentPropertiesSection(DAVA::UIControl* aControl, DAVA::UIComponent::eType type, int32 _index, const ComponentPropertiesSection* sourceSection, eCloneType cloneType)
    : SectionProperty("")
    , control(SafeRetain(aControl))
    , component(nullptr)
    , index(_index)
    , prototypeSection(nullptr) // weak
{
    component = UIComponent::CreateByType(type);
    DVASSERT(component);

    if (sourceSection && cloneType == CT_INHERIT)
    {
        prototypeSection = sourceSection; // weak
    }

    RefreshName();

    Reflection componentRef = Reflection::Create(&component);
    Vector<Reflection::Field> fields = componentRef.GetFields();
    for (const Reflection::Field &field : fields)
    {
        String name = field.key.Get<String>();
        const IntrospectionProperty* sourceProp = sourceSection == nullptr ? nullptr : sourceSection->FindChildPropertyByName(name);
        IntrospectionProperty* prop = new IntrospectionProperty(component, name, field.ref, sourceProp, cloneType);
        AddProperty(prop);
        SafeRelease(prop);
    }
}

ComponentPropertiesSection::~ComponentPropertiesSection()
{
    SafeRelease(control);
    SafeRelease(component);
    prototypeSection = nullptr; // weak
}

UIComponent* ComponentPropertiesSection::GetComponent() const
{
    return component;
}

DAVA::uint32 ComponentPropertiesSection::GetComponentType() const
{
    return component->GetType();
}

void ComponentPropertiesSection::AttachPrototypeSection(ComponentPropertiesSection* section)
{
    if (prototypeSection == nullptr)
    {
        prototypeSection = section;
        
        Reflection componentRef = Reflection::Create(&component);
        Vector<Reflection::Field> fields = componentRef.GetFields();

        for (const Reflection::Field &field : fields)
        {
            String name = field.key.Get<String>();
            ValueProperty* value = FindChildPropertyByName(name);
            ValueProperty* prototypeValue = prototypeSection->FindChildPropertyByName(name);
            value->AttachPrototypeProperty(prototypeValue);
        }
    }
    else
    {
        DVASSERT(false);
    }
}

void ComponentPropertiesSection::DetachPrototypeSection(ComponentPropertiesSection* section)
{
    if (prototypeSection == section)
    {
        prototypeSection = nullptr; // weak
        for (uint32 i = 0; i < GetCount(); i++)
        {
            ValueProperty* value = GetProperty(i);
            if (value->GetPrototypeProperty())
            {
                DVASSERT(value->GetPrototypeProperty()->GetParent() == section);
                value->DetachPrototypeProperty(value->GetPrototypeProperty());
            }
            else
            {
                DVASSERT(false);
            }
        }
    }
    else
    {
        DVASSERT(false);
    }
}

bool ComponentPropertiesSection::HasChanges() const
{
    return SectionProperty::HasChanges() || (GetFlags() & AbstractProperty::EF_INHERITED) == 0;
}

uint32 ComponentPropertiesSection::GetFlags() const
{
    bool readOnly = IsReadOnly();

    uint32 flags = 0;

    if (!readOnly && prototypeSection == nullptr)
        flags |= EF_CAN_REMOVE;

    if (prototypeSection)
        flags |= EF_INHERITED;

    return flags;
}

void ComponentPropertiesSection::InstallComponent()
{
    if (control->GetComponent(component->GetType(), 0) != component)
    {
        control->InsertComponentAt(component, index);
    }
}

void ComponentPropertiesSection::UninstallComponent()
{
    UIComponent* installedComponent = control->GetComponent(component->GetType(), index);
    if (installedComponent)
    {
        DVASSERT(installedComponent == component);
        control->RemoveComponent(component);
    }
}

int32 ComponentPropertiesSection::GetComponentIndex() const
{
    return index;
}

void ComponentPropertiesSection::RefreshIndex()
{
    if (component->GetControl() == control)
    {
        index = control->GetComponentIndex(component);
        RefreshName();
    }
}

void ComponentPropertiesSection::Accept(PropertyVisitor* visitor)
{
    visitor->VisitComponentSection(this);
}

String ComponentPropertiesSection::GetComponentName() const
{
    return GlobalEnumMap<UIComponent::eType>::Instance()->ToString(component->GetType());
}

void ComponentPropertiesSection::RefreshName()
{
    name = GetComponentName();
    if (UIComponent::IsMultiple(component->GetType()))
        name += Format(" [%d]", index);
}
