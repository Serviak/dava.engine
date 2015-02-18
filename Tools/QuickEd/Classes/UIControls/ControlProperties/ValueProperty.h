#ifndef __UI_EDITOR_VALUE_PROPERTY__
#define __UI_EDITOR_VALUE_PROPERTY__

#include "BaseProperty.h"

class SubValueProperty;

class ValueProperty : public BaseProperty
{
public:
    ValueProperty(DAVA::BaseObject *object, const DAVA::InspMember *member, ValueProperty *sourceProperty, eCopyType copyType);
    
protected:
    virtual ~ValueProperty();
    
public:
    virtual int GetCount() const override;
    virtual BaseProperty *GetProperty(int index) const override;

    virtual bool HasChanges() const override;
    virtual void Serialize(PackageSerializer *serializer) const override;

    virtual DAVA::String GetName() const;
    virtual ePropertyType GetType() const;
    virtual eEditFrags GetEditFlag() const { return EF_CAN_RESET; };

    virtual DAVA::VariantType GetValue() const;
    virtual void SetValue(const DAVA::VariantType &newValue);
    virtual void ResetValue();
    virtual bool IsReplaced() const;
    
    virtual DAVA::String GetSubValueName(int index) const;
    virtual DAVA::VariantType GetSubValue(int index) const;
    virtual void SetSubValue(int index, const DAVA::VariantType &newValue);
    
    virtual DAVA::BaseObject *GetBaseObject() const {
        return object;
    }
    
    virtual const DAVA::InspMember *GetMember() const {
        return member;
    }
    
    virtual const EnumMap *GetEnumMap() const;

private:
    DAVA::BaseObject *object;
    const DAVA::InspMember *member;
    bool replaced;
    DAVA::VariantType defaultValue;
    DAVA::Vector<SubValueProperty*> children;
};

#endif //__UI_EDITOR_VALUE_PROPERTY__
