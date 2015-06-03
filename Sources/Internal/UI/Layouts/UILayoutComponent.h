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

#ifndef __DAVAENGINE_UI_LAYOUT_COMPONENT_H__
#define __DAVAENGINE_UI_LAYOUT_COMPONENT_H__

#include "UI/Components/UIComponent.h"

namespace DAVA
{
    class UILayoutComponent : public UIComponent
    {
    public:
        enum eLayoutType {
            LINEAR_LAYOUT,
            ALIGN_LAYOUT
        };
        
    public:
        IMPLEMENT_COMPONENT_TYPE(LAYOUT_COMPONENT);
        
        UILayoutComponent();
        UILayoutComponent(const UILayoutComponent &src);
        
    protected:
        virtual ~UILayoutComponent();
        
    private:
        UILayoutComponent &operator=(const UILayoutComponent &) = delete;
        
    public:
        virtual UILayoutComponent* Clone() override;
        
        eLayoutType GetLayoutType() const;
        void SetLayoutType(eLayoutType type);

    private:
        int32 GetLayoutTypeAsInt() const;
        void SetLayoutTypeFromInt(int32 type);

    private:
        eLayoutType type;
        
    public:
        INTROSPECTION_EXTEND(UILayoutComponent, UIComponent,
                             PROPERTY("layout", InspDesc("Layout", GlobalEnumMap<eLayoutType>::Instance()), GetLayoutTypeAsInt, SetLayoutTypeFromInt, I_SAVE | I_VIEW | I_EDIT)
                             );
        
    };
    
}


#endif //__DAVAENGINE_UI_FAKE_COMPONENT_H__
