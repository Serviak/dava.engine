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


#ifndef __DAVAENGINE_UI_DEFAULT_PACKAGE_LOADER_H__
#define __DAVAENGINE_UI_DEFAULT_PACKAGE_LOADER_H__

#include "AbstractUIPackageBuilder.h"
#include "UIPackage.h"

namespace DAVA
{
    class DefaultUIPackageBuilder : public AbstractUIPackageBuilder
    {
    public:
        DefaultUIPackageBuilder();
        virtual ~DefaultUIPackageBuilder();
        
        virtual UIPackage *BeginPackage(const FilePath &packagePath) override;
        virtual void EndPackage() override;
        
        virtual UIPackage *ProcessImportedPackage(const String &packagePath, AbstractUIPackageLoader *loader) override;
        
        virtual UIControl *BeginControlWithClass(const String &className) override;
        virtual UIControl *BeginControlWithCustomClass(const String &customClassName, const String &className) override;
        virtual UIControl *BeginControlWithPrototype(const String &packageName, const String &prototypeName, const String &customClassName, AbstractUIPackageLoader *loader) override;
        virtual UIControl *BeginControlWithPath(const String &pathName) override;
        virtual UIControl *BeginUnknownControl(const YamlNode *node) override;
        virtual void EndControl() override;
        
        virtual void BeginControlPropretiesSection(const String &name) override;
        virtual void EndControlPropertiesSection() override;
        
        virtual UIControlBackground *BeginBgPropertiesSection(int index, bool sectionHasProperties) override;
        virtual void EndBgPropertiesSection() override;
        
        virtual UIControl *BeginInternalControlSection(int index, bool sectionHasProperties) override;
        virtual void EndInternalControlSection() override;
        
        virtual void ProcessProperty(const InspMember *member, const VariantType &value) override;
        
    private:
        struct ControlDescr {
            UIControl *control;
            bool addToParent;
            
            ControlDescr();
            ControlDescr(UIControl *node, bool addToParent);
            ControlDescr(const ControlDescr &descr);
            ~ControlDescr();
            ControlDescr &operator=(const ControlDescr &descr);
        };

        UIPackage *package;
        Map<String, UIPackage*> importedPackages;
        List<ControlDescr> controlsStack;
        BaseObject *currentObject;
        
    private:
        static const String EXCEPTION_CLASS_UI_TEXT_FIELD;
    };
}

#endif // __DAVAENGINE_UI_DEFAULT_PACKAGE_LOADER_H__
