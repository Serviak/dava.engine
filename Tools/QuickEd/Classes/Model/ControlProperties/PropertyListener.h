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


#ifndef __QUICKED_PROPERTY_LISTENER_H__
#define __QUICKED_PROPERTY_LISTENER_H__

class AbstractProperty;
class RootProperty;
class SectionProperty;
class ComponentPropertiesSection;
class StyleSheetProperty;
class StyleSheetSelectorProperty;

class PropertyListener
{
public:
    virtual void PropertyChanged(AbstractProperty *property) = 0;

    virtual void ComponentPropertiesWillBeAdded(RootProperty *root, ComponentPropertiesSection *section, int index) = 0;
    virtual void ComponentPropertiesWasAdded(RootProperty *root, ComponentPropertiesSection *section, int index) = 0;

    virtual void ComponentPropertiesWillBeRemoved(RootProperty *root, ComponentPropertiesSection *section, int index) = 0;
    virtual void ComponentPropertiesWasRemoved(RootProperty *root, ComponentPropertiesSection *section, int index) = 0;
    
    virtual void StylePropertyWillBeAdded(SectionProperty *section, StyleSheetProperty *property, int index) = 0;
    virtual void StylePropertyWasAdded(SectionProperty *section, StyleSheetProperty *property, int index) = 0;

    virtual void StylePropertyWillBeRemoved(SectionProperty *section, StyleSheetProperty *property, int index) = 0;
    virtual void StylePropertyWasRemoved(SectionProperty *section, StyleSheetProperty *property, int index) = 0;

    virtual void StyleSelectorWillBeAdded(SectionProperty *section, StyleSheetSelectorProperty *property, int index) = 0;
    virtual void StyleSelectorWasAdded(SectionProperty *section, StyleSheetSelectorProperty *property, int index) = 0;
    
    virtual void StyleSelectorWillBeRemoved(SectionProperty *section, StyleSheetSelectorProperty *property, int index) = 0;
    virtual void StyleSelectorWasRemoved(SectionProperty *section, StyleSheetSelectorProperty *property, int index) = 0;


};

#endif // __QUICKED_PROPERTY_LISTENER_H__
