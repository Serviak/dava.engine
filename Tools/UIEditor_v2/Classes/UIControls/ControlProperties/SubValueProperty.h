//
//  SubValueProperty.h
//  UIEditor
//
//  Created by Dmitry Belsky on 30.9.14.
//
//

#ifndef __UI_EDITOR_SUB_VALUE_PROPERTY__
#define __UI_EDITOR_SUB_VALUE_PROPERTY__

#include "BaseProperty.h"

class ValueProperty;

class SubValueProperty;

class SubValueProperty : public BaseProperty
{
public:
    SubValueProperty(int index);
    virtual ~SubValueProperty();

    virtual int GetCount() const override;
    virtual BaseProperty *GetProperty(int index) const override;

    virtual DAVA::String GetName() const;
    virtual ePropertyType GetType() const;
    virtual DAVA::VariantType GetValue() const;
    virtual void SetValue(const DAVA::VariantType &newValue);
    virtual bool IsReplaced() const;

private:
    ValueProperty *GetValueProperty() const;
    int index;
};

#endif // __UI_EDITOR_SUB_VALUE_PROPERTY__
