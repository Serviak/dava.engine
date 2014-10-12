#ifndef __UI_EDITOR_CONTROL_PROPERTIES_SECTION_H__
#define __UI_EDITOR_CONTROL_PROPERTIES_SECTION_H__

#include "PropertiesSection.h"

class ControlPropertiesSection : public PropertiesSection
{
public:
    ControlPropertiesSection(DAVA::UIControl *control, const DAVA::String &name);
    virtual ~ControlPropertiesSection();
    
    virtual DAVA::String GetName() const;
    ControlPropertiesSection *CopyAndApplyToOtherControl(DAVA::UIControl *control);

private:
    DAVA::UIControl *control;
    DAVA::String name;
};

#endif // __UI_EDITOR_CONTROL_PROPERTIES_SECTION_H__
