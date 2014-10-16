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

#include "UI/UIControl.h"
#include "UI/UIControlSystem.h"
#include "UI/UIYamlLoader.h"
#include "UI/UIControlHelpers.h"
#include "Animation/LinearAnimation.h"
#include "Debug/DVAssert.h"
#include "FileSystem/YamlNode.h"
#include "Input/InputSystem.h"
#include "Render/RenderHelper.h"
#include "Render/RenderManager.h"
#include "Utils/StringFormat.h"

namespace DAVA
{

    UIControl::UIControl(const Rect &rect, bool rectInAbsoluteCoordinates/* = false*/)
        : customData(NULL)
    {
        parent = NULL;
        controlState = STATE_NORMAL;
        recursiveVisible = true;
        visible = true;
        visibleForUIEditor = true;
        /*
            VB:
            please do not change anymore to false, it no make any sense to make all controls untouchable by default.
            for particular controls it can be changed, but no make sense to make that for all controls.
         */
        inputEnabled = true;
        inputProcessorsCount = 1;
        focusEnabled = true;

        background = new UIControlBackground();
        eventDispatcher = NULL;
        clipContents = false;

        debugDrawEnabled = false;
        debugDrawColor = Color(1.0f, 0.0f, 0.0f, 1.0f);

        drawPivotPointMode = DRAW_NEVER;

        pivotPoint = Vector2(0, 0);
        scale = Vector2(1.0f, 1.0f);
        angle = 0;

        leftAlign = 0;
        hcenterAlign = 0;
        rightAlign = 0;
        topAlign = 0;
        vcenterAlign = 0;
        bottomAlign = 0;

        leftAlignEnabled = false;
        hcenterAlignEnabled = false;
        rightAlignEnabled = false;
        topAlignEnabled = false;
        vcenterAlignEnabled = false;
        bottomAlignEnabled = false;

        tag = 0;

        multiInput = false;
        exclusiveInput = false;
        currentInputID = 0;
        touchesInside = 0;
        totalTouches = 0;


        SetRect(rect, rectInAbsoluteCoordinates);

        initialState = STATE_NORMAL;
    }

    UIControl::~UIControl()
    {
        UIControlSystem::Instance()->CancelInputs(this);
        SafeRelease(background);
        SafeRelease(eventDispatcher);
        RemoveAllControls();
        SafeRelease(customData);
    }

    void UIControl::SetParent(UIControl *newParent)
    {
        if (parent)
        {
            parent->UnregisterInputProcessors(inputProcessorsCount);
        }
        parent = newParent;
        if (parent)
        {
            parent->RegisterInputProcessors(inputProcessorsCount);
        }
    }
    UIControl *UIControl::GetParent() const
    {
        return parent;
    }

    void UIControl::SetExclusiveInput(bool isExclusiveInput, bool hierarchic/* = true*/)
    {
        exclusiveInput = isExclusiveInput;

        if(hierarchic)
        {
            List<UIControl*>::iterator it = childs.begin();
            for(; it != childs.end(); ++it)
            {
                (*it)->SetExclusiveInput(isExclusiveInput, hierarchic);
            }
        }
    }

    void UIControl::SetMultiInput(bool isMultiInput, bool hierarchic/* = true*/)
    {
        multiInput = isMultiInput;

        if(hierarchic)
        {
            List<UIControl*>::iterator it = childs.begin();
            for(; it != childs.end(); ++it)
            {
                (*it)->SetMultiInput(isMultiInput, hierarchic);
            }
        }
    }


    void UIControl::AddEvent(int32 eventType, const Message &msg)
    {
        if(!eventDispatcher)
        {
            eventDispatcher = new EventDispatcher();
        }
        eventDispatcher->AddEvent(eventType, msg);
    }
    bool UIControl::RemoveEvent(int32 eventType, const Message &msg)
    {
        if(eventDispatcher)
        {
            return eventDispatcher->RemoveEvent(eventType, msg);
        }
        return false;
    }

    bool UIControl::RemoveAllEvents()
    {
        if(eventDispatcher)
        {
            return eventDispatcher->RemoveAllEvents();
        }
        return false;
    }

    void UIControl::PerformEvent(int32 eventType)
    {
        if(eventDispatcher)
        {
            eventDispatcher->PerformEvent(eventType, this);
        }
    }

    void UIControl::PerformEventWithData(int32 eventType, void *callerData)
    {
        if(eventDispatcher)
        {
            eventDispatcher->PerformEventWithData(eventType, this, callerData);
        }
    }


    const List<UIControl*> & UIControl::GetChildren() const
    {
        return childs;
    }

    List<UIControl* >& UIControl::GetRealChildren()
    {
        realChilds.clear();
        realChilds = childs;

        return realChilds;
    }

    List<UIControl* > UIControl::GetSubcontrols()
    {
        // Default list of Subcontrols is empty. To be overriden in the derived
        // controls.
        return List<UIControl*>();
    }

    bool UIControl::IsSubcontrol()
    {
        if (!this->GetParent())
        {
            return false;
        }

        const List<UIControl*>& parentSubcontrols = parent->GetSubcontrols();
        if (parentSubcontrols.empty())
        {
            return false;
        }

        bool isSubcontrol = (std::find(parentSubcontrols.begin(), parentSubcontrols.end(), this) != parentSubcontrols.end());
        return isSubcontrol;
    }

    bool UIControl::AddControlToList(List<UIControl*>& controlsList, const String& controlName, bool isRecursive)
    {
        UIControl* control = FindByName(controlName, isRecursive);
        if (control)
        {
            controlsList.push_back(control);
            return true;
        }

        return false;
    }

    void UIControl::SetName(const String & _name)
    {
        name = _name;
    }

    void UIControl::SetTag(int32 _tag)
    {
        tag = _tag;
    }

    // return first control with given name
    UIControl * UIControl::FindByName(const String & name, bool recursive) const
    {
        List<UIControl*>::const_iterator it = childs.begin();
        for(; it != childs.end(); ++it)
        {
            UIControl * c = (*it);
            if (c->name == name)return c;

            if (recursive)
            {
                UIControl * inChilds = c->FindByName(name);
                if (inChilds)return inChilds;
            }
        }
        return 0;
    }

    UIControl * UIControl::FindByPath(const String &path) const
    {
        return UIControlHelpers::GetControlByPath(path, this);
    }

    void UIControl::SetState(int32 state)
    {
        controlState = state;
    }

    Sprite* UIControl::GetSprite() const
    {
        return background->GetSprite();
    }

    int32 UIControl::GetFrame() const
    {
        return background->GetFrame();
    }

    UIControlBackground::eDrawType UIControl::GetSpriteDrawType() const
    {
        return background->GetDrawType();
    }
    int32 UIControl::GetSpriteAlign() const
    {
        return background->GetAlign();
    }
    void UIControl::SetSprite(const FilePath &spriteName, int32 spriteFrame)
    {
        background->SetSprite(spriteName, spriteFrame);
    }
    void UIControl::SetSprite(Sprite *newSprite, int32 spriteFrame)
    {
        background->SetSprite(newSprite, spriteFrame);
    }
    void UIControl::SetSpriteFrame(int32 spriteFrame)
    {
        background->SetFrame(spriteFrame);
    }
    void UIControl::SetSpriteDrawType(UIControlBackground::eDrawType drawType)
    {
        background->SetDrawType(drawType);
    }
    void UIControl::SetSpriteAlign(int32 align)
    {
        background->SetAlign(align);
    }

    void UIControl::SetLeftAlign(int32 align)
    {
        // Set a property value
        leftAlign = align;

        if (!leftAlignEnabled)
            return;

        // Get current right and horizontal align
        int32 rightAlign = this->GetRightAlign();
        int32 hcenterAlign = this->GetHCenterAlign();

        // Get the relative position of control
        Vector2 relativePosition = this->GetPosition(false);

        // Change the size of control if other align option is set.
        // We need a proper parent for this operation
        UIControl *parentControl = this->GetParent();
        Vector2 controlSize = this->GetSize();

        if (!parentControl)
            return;

        // Get the new size of control
        if (this->GetRightAlignEnabled())
        {
            controlSize.x = GetSizeX(parentControl, align, rightAlign);
        }
        else if (this->GetHCenterAlignEnabled())
        {
            controlSize.x = GetSizeX(parentControl, align, (-1)*hcenterAlign, true);
        }
        SetSize(controlSize);
        // Change position
        relativePosition.x = (float32)align + GetPivotPoint().x;
        // Update the size of control
        SetPosition(relativePosition, false);
    }

    int32 UIControl::GetLeftAlign() const
    {
        return leftAlign;
    }

    void UIControl::SetHCenterAlign(int32 align)
    {
        hcenterAlign = align;

        if (!hcenterAlignEnabled)
            return;

        UIControl *parentControl = this->GetParent();
        if (!parentControl)
            return;

        int32 leftAlign = this->GetLeftAlign();
        int32 rightAlign = this->GetRightAlign();
        Vector2 relativePosition = this->GetPosition(false);
        Vector2 controlSize = this->GetSize();

        // Check if two align options selected simultaneously
        if (GetLeftAlignEnabled())
        {
            controlSize.x = GetSizeX(parentControl, (-1)*align, leftAlign, true);
        }
        else if (GetRightAlignEnabled())
        {
            controlSize.x = GetSizeX(parentControl, align, rightAlign, true);
        }

        SetSize(controlSize);

        if (GetLeftAlignEnabled())
        {
            relativePosition.x = (float32)leftAlign + GetPivotPoint().x;
        }
        else if (GetRightAlignEnabled())
        {
            relativePosition.x = GetRelativeX(parentControl, align) + GetPivotPoint().x;
        }
        else // If only hcenter option is selected - set center of this control relative to it's parent center
        {
            relativePosition.x = GetCenterX(parentControl, align, this) + GetPivotPoint().x;
        }

        SetPosition(relativePosition);
    }

    int32 UIControl::GetHCenterAlign() const
    {
        return hcenterAlign;
    }

    void UIControl::SetRightAlign(int32 align)
    {
        rightAlign = align;

        if (!rightAlignEnabled)
            return;

        // Get current right and horizontal align
        int32 leftAlign = this->GetLeftAlign();
        int32 hcenterAlign = this->GetHCenterAlign();
        // Get current position and size
        Vector2 relativePosition = this->GetPosition(false);
        UIControl *parentControl = this->GetParent();
        Vector2 controlSize = this->GetSize();

        if (!parentControl)
            return;

        if (GetLeftAlignEnabled())
        {
            controlSize.x = GetSizeX(parentControl, leftAlign, align);
        }
        else if (GetHCenterAlignEnabled())
        {
            controlSize.x = GetSizeX(parentControl, align, hcenterAlign, true);
        }

        SetSize(controlSize);

        if (GetLeftAlignEnabled())
        {
            relativePosition.x = (float32)leftAlign + GetPivotPoint().x;
        }
        else if (GetHCenterAlignEnabled())
        {
            relativePosition.x =  GetRelativeX(parentControl, hcenterAlign) + GetPivotPoint().x;
        }
        else // If only right option is on - just change relative position
        {
            relativePosition.x = GetRelativeX(parentControl, align, this) + GetPivotPoint().x;
        }
        SetPosition(relativePosition);
    }

    int32 UIControl::GetRightAlign() const
    {
        return rightAlign;
    }

    void UIControl::SetTopAlign(int32 align)
    {
        topAlign = align;

        if (!topAlignEnabled)
            return;

        //Get current top and vertical align
        int32 bottomAlign = this->GetBottomAlign();
        int32 vcenterAlign = this->GetVCenterAlign();

        // Get relative position of control
        Vector2 relativePosition = this->GetPosition(false);

        UIControl *parentControl = this->GetParent();
        Vector2 controlSize = this->GetSize();

        if (!parentControl)
            return;

        if (this->GetBottomAlignEnabled())
        {
            controlSize.y = GetSizeY(parentControl, align, bottomAlign);
        }
        else if (this->GetVCenterAlignEnabled())
        {
            controlSize.y = GetSizeY(parentControl, align, (-1)*vcenterAlign, true);
        }
        SetSize(controlSize);
        // Set position and change size
        relativePosition.y = (float32)align + GetPivotPoint().y;
        this->SetPosition(relativePosition, false);
        
    }

    int32 UIControl::GetTopAlign() const
    {
        return topAlign;
    }

    void UIControl::SetVCenterAlign(int32 align)
    {
        vcenterAlign = align;

        if (!vcenterAlignEnabled)
            return;

        UIControl *parentControl = this->GetParent();
        if (!parentControl)
            return;

        int32 topAlign = this->GetTopAlign();
        int32 bottomAlign = this->GetBottomAlign();
        Vector2 relativePosition = this->GetPosition(false);
        Vector2 controlSize = this->GetSize();

        // Check if two align options selected simultaneously
        if (GetTopAlignEnabled())
        {
            controlSize.y = GetSizeY(parentControl, (-1)*align, topAlign, true);
        }
        else if (GetBottomAlignEnabled())
        {
            controlSize.y = GetSizeY(parentControl, align, bottomAlign, true);
        }

        SetSize(controlSize);

        if (GetTopAlignEnabled())
        {
            relativePosition.y = (float32)topAlign + GetPivotPoint().y;
        }
        else if (GetBottomAlignEnabled())
        {
            relativePosition.y = GetRelativeY(parentControl, align) + GetPivotPoint().y;
        }
        else
        {
            relativePosition.y = GetCenterY(parentControl, align, this) + GetPivotPoint().y;
        }

        this->SetPosition(relativePosition);
    }

    int32 UIControl::GetVCenterAlign() const
    {
        return vcenterAlign;
    }

    void UIControl::SetBottomAlign(int32 align)
    {
        bottomAlign = align;

        if (!bottomAlignEnabled)
            return;

        // Get current right and horizontal align
        int32 topAlign = this->GetTopAlign();
        int32 vcenterAlign = this->GetVCenterAlign();

        // Get current positon and size
        Vector2 relativePosition = this->GetPosition(false);
        Vector2 controlSize = this->GetSize();
        UIControl *parentControl = this->GetParent();

        if (!parentControl)
            return;

        if (GetTopAlignEnabled())
        {
            controlSize.y = GetSizeY(parentControl, topAlign, align);
        }
        else if (GetVCenterAlignEnabled())
        {
            controlSize.y = GetSizeY(parentControl, vcenterAlign, align, true);
        }

        SetSize(controlSize);

        if (GetTopAlignEnabled())
        {
            relativePosition.y = (float32)topAlign + GetPivotPoint().y;
        }
        else if (GetVCenterAlignEnabled())
        {
            relativePosition.y =  GetRelativeY(parentControl, vcenterAlign) + GetPivotPoint().y;
        }
        else //If only bottom option is on - just change relative position
        {
            relativePosition.y = GetRelativeY(parentControl, align, this) + GetPivotPoint().y;
        }

        SetPosition(relativePosition);
    }

    int32 UIControl::GetBottomAlign() const
    {
        return bottomAlign;
    }

    // Enable align options methods
    void UIControl::SetLeftAlignEnabled(bool isEnabled)
    {
        leftAlignEnabled = isEnabled;
        if (isEnabled)
        {
            SetLeftAlign(leftAlign);
        }
        else
        {
            if (GetHCenterAlignEnabled())
            {
                SetHCenterAlign(hcenterAlign);
            }
        }
    }

    bool UIControl::GetLeftAlignEnabled() const
    {
        return leftAlignEnabled;
    }

    void UIControl::SetHCenterAlignEnabled(bool isEnabled)
    {
        hcenterAlignEnabled = isEnabled;
        if (isEnabled)
        {
            SetHCenterAlign(hcenterAlign);
        }
    }

    bool UIControl::GetHCenterAlignEnabled() const
    {
        return hcenterAlignEnabled;
    }

    void UIControl::SetRightAlignEnabled(bool isEnabled)
    {
        rightAlignEnabled = isEnabled;
        if (isEnabled)
        {
            SetRightAlign(rightAlign);
        }
        else
        {
            if (GetHCenterAlignEnabled())
            {
                SetHCenterAlign(hcenterAlign);
            }
        }
    }

    bool UIControl::GetRightAlignEnabled() const
    {
        return rightAlignEnabled;
    }

    void UIControl::SetTopAlignEnabled(bool isEnabled)
    {
        topAlignEnabled = isEnabled;
        if(isEnabled)
        {
            SetTopAlign(topAlign);
        }
        else
        {
            if (GetVCenterAlignEnabled())
            {
                SetVCenterAlign(vcenterAlign);
            }
        }
    }

    bool UIControl::GetTopAlignEnabled() const
    {
        return topAlignEnabled;
    }

    void UIControl::SetVCenterAlignEnabled(bool isEnabled)
    {
        vcenterAlignEnabled = isEnabled;
        if (isEnabled)
        {
            SetVCenterAlign(vcenterAlign);
        }
    }

    bool UIControl::GetVCenterAlignEnabled() const
    {
        return vcenterAlignEnabled;
    }

    void UIControl::SetBottomAlignEnabled(bool isEnabled)
    {
        bottomAlignEnabled = isEnabled;
        if (isEnabled)
        {
            SetBottomAlign(bottomAlign);
        }
        else
        {
            if (GetVCenterAlignEnabled())
            {
                SetVCenterAlign(vcenterAlign);
            }
        }
    }

    bool UIControl::GetBottomAlignEnabled() const
    {
        return bottomAlignEnabled;
    }

    void UIControl::SetBackground(UIControlBackground *newBg)
    {
        DVASSERT(newBg);
        SafeRelease(background);
        background = newBg->Clone();
    }

    UIControlBackground *UIControl::GetBackground() const
    {
        return background;
    }

    const UIGeometricData &UIControl::GetGeometricData()
    {
        tempGeometricData.position = relativePosition;
        tempGeometricData.size = size;
        tempGeometricData.pivotPoint = pivotPoint;
        tempGeometricData.scale = scale;
        tempGeometricData.angle = angle;
        if(!parent)
        {
            tempGeometricData.AddGeometricData(UIControlSystem::Instance()->GetBaseGeometricData());
            return tempGeometricData;
        }
        tempGeometricData.AddGeometricData(parent->GetGeometricData());
        return tempGeometricData;
    }

    UIGeometricData UIControl::GetLocalGeometricData() const
    {
        UIGeometricData drawData;
        drawData.position = relativePosition;
        drawData.size = size;
        drawData.pivotPoint = pivotPoint;
        drawData.scale = scale;
        drawData.angle = angle;

        return drawData;
    }

    Vector2 UIControl::GetPosition(bool absoluteCoordinates)
    {
        if(!absoluteCoordinates || !parent)
        {
            return GetPosition();
        }

        return GetAbsolutePosition();
    }

    Vector2 UIControl::GetAbsolutePosition()
    {
        return GetGeometricData().position;
    }

    void UIControl::SetPosition(const Vector2 &position)
    {
        relativePosition = position;
    }

    void UIControl::SetPosition(const Vector2 &position, bool positionInAbsoluteCoordinates)
    {
        if(!positionInAbsoluteCoordinates)
        {
            SetPosition(position);
        }
        else
        {
            SetAbsolutePosition(position);
        }
    }

    void UIControl::SetAbsolutePosition(const Vector2 &position)
    {
        if(parent)
        {
            const UIGeometricData &parentGD = parent->GetGeometricData();
            SetPosition(position - parentGD.position + parentGD.pivotPoint);
        }
        else
        {
            SetPosition(position);
        }
    }

    void UIControl::SetSize(const Vector2 &newSize)
    {
        Vector2 oldPivot = GetPivot();
        size = newSize;
        SetPivot(oldPivot);
        // Update size and align of childs
        UpdateLayout();
    }

    float32 UIControl::GetParentsTotalAngle(bool includeOwn)
    {
        float32 angle = 0;
        if(includeOwn)
        {
            angle += this->angle;
        }
        if(this->GetParent())
        {
            angle += parent->GetParentsTotalAngle(true);
        }
        return angle;
    }

    void UIControl::SetAngle(float32 angleInRad)
    {
        angle = angleInRad;
    }
    
    void UIControl::SetAngleInDegrees(float32 angleInDeg)
    {
        SetAngle(DegToRad(angleInDeg));
    }

    Rect UIControl::GetRect(bool absoluteCoordinates)
    {
        if(!absoluteCoordinates)
            return GetRect();

        return GetAbsoluteRect();
    }

    Rect UIControl::GetAbsoluteRect()
    {
        return Rect(GetAbsolutePosition() - GetPivotPoint(), size);
    }

    void UIControl::SetRect(const Rect &rect)
    {
        SetSize(rect.GetSize());
        SetPosition(rect.GetPosition() + GetPivotPoint());

        // Update aligns if control was resized manually
        RecalculateAlignProperties();
    }

    void UIControl::SetAbsoluteRect(const Rect &rect)
    {
        if (!parent)
        {
            SetRect(rect);
            return;
        }

        Rect localRect = rect;
        const UIGeometricData &parentGD = parent->GetGeometricData();
        localRect.SetPosition(rect.GetPosition() - parentGD.position + parentGD.pivotPoint);
        SetRect(localRect);
    }

    void UIControl::SetRect(const Rect &rect, bool rectInAbsoluteCoordinates/* = false*/)
    {
        if (!rectInAbsoluteCoordinates)
        {
            SetRect(rect);
        }
        else
        {
            SetAbsoluteRect(rect);
        }
    }

    void UIControl::SetScaledRect(const Rect &rect, bool rectInAbsoluteCoordinates/* = false*/)
    {
        if(!rectInAbsoluteCoordinates || !parent)
        {
            scale.x = rect.dx / size.x;
            scale.y = rect.dy / size.y;
            SetPosition(Vector2(rect.x + pivotPoint.x * scale.x, rect.y + pivotPoint.y * scale.y), rectInAbsoluteCoordinates);
        }
        else
        {
            const UIGeometricData &gd = parent->GetGeometricData();
            scale.x = rect.dx / (size.x * gd.scale.x);
            scale.y = rect.dy / (size.y * gd.scale.y);
            SetPosition(Vector2(rect.x + pivotPoint.x * scale.x, rect.y + pivotPoint.y * scale.y), rectInAbsoluteCoordinates);
        }
    }


    bool UIControl::GetVisible() const
    {
        return visible;
    }

    void UIControl::SetVisible(bool isVisible, bool hierarchic/* = true*/)
    {
        if (!isVisible && visible)
        {
            UIControlSystem::Instance()->CancelInputs(this);
        }
        visible = isVisible;

        if(hierarchic)
        {
            List<UIControl*>::iterator it = childs.begin();
            for(; it != childs.end(); ++it)
            {
                (*it)->SetVisible(isVisible, hierarchic);
            }
        }
    }

    void UIControl::SetRecursiveVisible(bool isVisible)
    {
        if (recursiveVisible == isVisible)
            return;

        recursiveVisible = isVisible;

        if (parent && parent->IsOnScreen())
        {
            if (recursiveVisible)
                SystemWillBecomeVisible();
            else
                SystemWillBecomeInvisible();
        }
    }

    void UIControl::SetVisibleForUIEditor(bool value, bool hierarchic/* = true*/)
    {
        visibleForUIEditor = value;
        if(hierarchic)
        {
            List<UIControl*>::iterator it = childs.begin();
            for(; it != childs.end(); ++it)
            {
                (*it)->SetVisibleForUIEditor(value, hierarchic);
            }
        }
    }

    void UIControl::SetInputEnabled(bool isEnabled, bool hierarchic/* = true*/)
    {
        if (isEnabled != inputEnabled)
        {
            inputEnabled = isEnabled;
            if (inputEnabled)
            {
                RegisterInputProcessor();
            }
            else
            {
                UnregisterInputProcessor();
            }
        }
        if(hierarchic)
        {
            List<UIControl*>::iterator it = childs.begin();
            for(; it != childs.end(); ++it)
            {
                (*it)->SetInputEnabled(isEnabled, hierarchic);
            }
        }
    }

    void UIControl::SetFocusEnabled(bool isEnabled)
    {
        focusEnabled = isEnabled;
    }

    bool UIControl::GetDisabled() const
    {
        return ((controlState & STATE_DISABLED) != 0);
    }

    void UIControl::SetDisabled(bool isDisabled, bool hierarchic/* = true*/)
    {
        if(isDisabled)
        {
            controlState |= STATE_DISABLED;

            // Cancel all inputs because of DF-2943.
            UIControlSystem::Instance()->CancelInputs(this);
        }
        else
        {
            controlState &= ~STATE_DISABLED;
        }

        if(hierarchic)
        {
            List<UIControl*>::iterator it = childs.begin();
            for(; it != childs.end(); ++it)
            {
                (*it)->SetDisabled(isDisabled, hierarchic);
            }
        }
    }

    bool UIControl::GetSelected() const
    {
        return ((controlState & STATE_SELECTED) != 0);
    }

    void UIControl::SetSelected(bool isSelected, bool hierarchic/* = true*/)
    {
        if(isSelected)
        {
            controlState |= STATE_SELECTED;
        }
        else
        {
            controlState &= ~STATE_SELECTED;
        }

        if(hierarchic)
        {
            List<UIControl*>::iterator it = childs.begin();
            for(; it != childs.end(); ++it)
            {
                (*it)->SetSelected(isSelected, hierarchic);
            }
        }
    }

    void UIControl::SetClipContents(bool isNeedToClipContents)
    {
        clipContents = isNeedToClipContents;
    }

    bool UIControl::GetHover() const
    {
        return (controlState & STATE_HOVER) != 0;
    }

    void UIControl::AddControl(UIControl *control)
    {
        control->Retain();
        control->RemoveFromParent();

        bool inHierarchy = InViewHierarchy();
        if (inHierarchy)
        {
            control->SystemWillAppear();
        }
        control->isUpdated = false;
        control->SetParent(this);
        childs.push_back(control);
        if (inHierarchy)
        {
            control->SystemDidAppear();
        }

        if (IsOnScreen() && control->GetRecursiveVisible())
            control->SystemWillBecomeVisible();

        isIteratorCorrupted = true;
    }
    void UIControl::RemoveControl(UIControl *control)
    {
        if (NULL == control)
        {
            return;
        }

        List<UIControl*>::iterator it = childs.begin();
        for(; it != childs.end(); ++it)
        {
            if((*it) == control)
            {
                if (IsOnScreen() && control->GetRecursiveVisible())
                    control->SystemWillBecomeInvisible();

                bool inHierarchy = InViewHierarchy();
                if (inHierarchy)
                {
                    control->SystemWillDisappear();
                }
                control->SetParent(NULL);
                childs.erase(it);
                if (inHierarchy)
                {
                    control->SystemDidDisappear();
                }
                control->Release();
                isIteratorCorrupted = true;
                return;
            }
        }
    }

    void UIControl::RemoveFromParent()
    {
        UIControl* parentControl = this->GetParent();
        if (parentControl)
        {
            parentControl->RemoveControl(this);
        }
    }

    void UIControl::RemoveAllControls()
    {
        while(!childs.empty())
        {
            RemoveControl(childs.front());
        }
    }
    void UIControl::BringChildFront(UIControl *_control)
    {
        List<UIControl*>::iterator it = childs.begin();
        for(; it != childs.end(); ++it)
        {
            if((*it) == _control)
            {
                childs.erase(it);
                childs.push_back(_control);
                isIteratorCorrupted = true;
                return;
            }
        }
    }
    void UIControl::BringChildBack(UIControl *_control)
    {
        List<UIControl*>::iterator it = childs.begin();
        for(; it != childs.end(); ++it)
        {
            if((*it) == _control)
            {
                childs.erase(it);
                childs.push_front(_control);
                isIteratorCorrupted = true;
                return;
            }
        }
    }

    void UIControl::InsertChildBelow(UIControl * control, UIControl * _belowThisChild)
    {
        List<UIControl*>::iterator it = childs.begin();
        for(; it != childs.end(); ++it)
        {
            if((*it) == _belowThisChild)
            {
                control->Retain();
                control->RemoveFromParent();

                bool inHierarchy = InViewHierarchy();
                if(inHierarchy)
                {
                    control->SystemWillAppear();
                }
                childs.insert(it, control);
                control->SetParent(this);
                if(inHierarchy)
                {
                    control->SystemDidAppear();
                }

                if (IsOnScreen() && control->GetRecursiveVisible())
                    control->SystemWillBecomeVisible();

                isIteratorCorrupted = true;
                return;
            }
        }

        AddControl(control);
    }
    void UIControl::InsertChildAbove(UIControl * control, UIControl * _aboveThisChild)
    {
        List<UIControl*>::iterator it = childs.begin();
        for(; it != childs.end(); ++it)
        {
            if((*it) == _aboveThisChild)
            {
                control->Retain();
                control->RemoveFromParent();

                bool inHierarchy = InViewHierarchy();
                if(inHierarchy)
                {
                    control->SystemWillAppear();
                }
                childs.insert(++it, control);
                control->SetParent(this);
                if(inHierarchy)
                {
                    control->SystemDidAppear();
                }

                if (IsOnScreen() && control->GetRecursiveVisible())
                    control->SystemWillBecomeVisible();

                isIteratorCorrupted = true;
                return;
            }
        }

        AddControl(control);
    }

    void UIControl::SendChildBelow(UIControl * _control, UIControl * _belowThisChild)
    {
        //TODO: Fix situation when controls not from this hierarchy

        // firstly find control in list and erase it
        List<UIControl*>::iterator it = childs.begin();
        for(; it != childs.end(); ++it)
        {
            if((*it) == _control)
            {
                childs.erase(it);
                isIteratorCorrupted = true;
                break;
            }
        }
        // after that find place where we should put the control and do that
        it = childs.begin();
        for(; it != childs.end(); ++it)
        {
            if((*it) == _belowThisChild)
            {
                childs.insert(it, _control);
                isIteratorCorrupted = true;
                return;
            }
        }
        DVASSERT_MSG(0, "Control _belowThisChild not found");
    }

    void UIControl::SendChildAbove(UIControl * _control, UIControl * _aboveThisChild)
    {
        //TODO: Fix situation when controls not from this hierarhy

        // firstly find control in list and erase it
        List<UIControl*>::iterator it = childs.begin();
        for(; it != childs.end(); ++it)
        {
            if((*it) == _control)
            {
                childs.erase(it);
                isIteratorCorrupted = true;
                break;
            }
        }
        // after that find place where we should put the control and do that
        it = childs.begin();
        for(; it != childs.end(); ++it)
        {
            if((*it) == _aboveThisChild)
            {
                childs.insert(++it, _control);
                isIteratorCorrupted = true;
                return;
            }
        }

        DVASSERT_MSG(0, "Control _aboveThisChild not found");
    }

    UIControl *UIControl::Clone()
    {
        UIControl *c = new UIControl(Rect(relativePosition.x, relativePosition.y, size.x, size.y));
        c->CopyDataFrom(this);
        return c;
    }

    void UIControl::CopyDataFrom(UIControl *srcControl)
    {
        relativePosition = srcControl->relativePosition;
        size = srcControl->size;
        pivotPoint = srcControl->pivotPoint;
        scale = srcControl->scale;
        angle = srcControl->angle;
        SafeRelease(background);
        background = srcControl->background->Clone();

        leftAlign = srcControl->leftAlign;
        hcenterAlign = srcControl->hcenterAlign;
        rightAlign = srcControl->rightAlign;
        topAlign = srcControl->topAlign;
        vcenterAlign = srcControl->vcenterAlign;
        bottomAlign = srcControl->bottomAlign;

        leftAlignEnabled = srcControl->leftAlignEnabled;
        hcenterAlignEnabled = srcControl->hcenterAlignEnabled;
        rightAlignEnabled = srcControl->rightAlignEnabled;
        topAlignEnabled = srcControl->topAlignEnabled;
        vcenterAlignEnabled = srcControl->vcenterAlignEnabled;
        bottomAlignEnabled = srcControl->bottomAlignEnabled;

        tag = srcControl->GetTag();
        name = srcControl->name;

        controlState = srcControl->controlState;
        recursiveVisible = srcControl->recursiveVisible;
        visible = srcControl->visible;
        visibleForUIEditor = srcControl->visibleForUIEditor;
        inputEnabled = srcControl->inputEnabled;
        clipContents = srcControl->clipContents;

        customControlType = srcControl->GetCustomControlClassName();
        initialState = srcControl->GetInitialState();
        drawPivotPointMode = srcControl->drawPivotPointMode;
        debugDrawColor = srcControl->debugDrawColor;
        debugDrawEnabled = srcControl->debugDrawEnabled;

        SafeRelease(eventDispatcher);
        if(srcControl->eventDispatcher)
        {
            eventDispatcher = new EventDispatcher();
            eventDispatcher->CopyDataFrom(srcControl->eventDispatcher);
        }

        RemoveAllControls();
        if (inputEnabled)
        {
            inputProcessorsCount = 1;
        }
        else
        {
            inputProcessorsCount = 0;
        }

        // Yuri Coder, 2012/11/30. Use Real Children List to avoid copying
        // unnecessary children we have on the for example UIButton.
        const List<UIControl*>& realChildren = srcControl->GetRealChildren();
        List<UIControl*>::const_iterator it = realChildren.begin();
        for(; it != realChildren.end(); ++it)
        {

            UIControl *c = (*it)->Clone();
            AddControl(c);
            c->Release();
        }
    }


    bool UIControl::InViewHierarchy() const
    {
        if (UIControlSystem::Instance()->GetScreen() == this ||
            UIControlSystem::Instance()->GetPopupContainer() == this)
        {
            return true;
        }

        if (parent)
            return parent->InViewHierarchy();

        return false;
    }


    bool UIControl::IsOnScreen() const
    {
        if(UIControlSystem::Instance()->GetScreen() == this ||
           UIControlSystem::Instance()->GetPopupContainer() == this)
        {
            return GetRecursiveVisible();
        }

        if( !GetRecursiveVisible() || !parent )
            return false;

        return parent->IsOnScreen();
    }


    void UIControl::SystemWillAppear()
    {
        WillAppear();

        List<UIControl*>::iterator it = childs.begin();
        while(it != childs.end())
        {
            isIteratorCorrupted = false;
            UIControl *current = *it;
            current->Retain();
            current->SystemWillAppear();
            current->Release();
            if(isIteratorCorrupted)
            {
                it = childs.begin();
                continue;
            }
            ++it;
        }
    }

    void UIControl::SystemWillDisappear()
    {
        List<UIControl*>::iterator it = childs.begin();
        while(it != childs.end())
        {
            isIteratorCorrupted = false;
            UIControl *current = *it;
            current->Retain();
            current->SystemWillDisappear();
            current->Release();
            if(isIteratorCorrupted)
            {
                it = childs.begin();
                continue;
            }
            ++it;
        }

        WillDisappear();
    }

    void UIControl::SystemDidAppear()
    {
        DidAppear();

        List<UIControl*>::iterator it = childs.begin();
        while(it != childs.end())
        {
            isIteratorCorrupted = false;
            UIControl *current = *it;
            current->Retain();
            current->SystemDidAppear();
            current->Release();
            if(isIteratorCorrupted)
            {
                it = childs.begin();
                continue;
            }
            ++it;
        }
    }

    void UIControl::SystemDidDisappear()
    {
        DidDisappear();

        List<UIControl*>::iterator it = childs.begin();
        while(it != childs.end())
        {
            isIteratorCorrupted = false;
            UIControl *current = *it;
            current->Retain();
            current->SystemDidDisappear();
            current->Release();
            if(isIteratorCorrupted)
            {
                it = childs.begin();
                continue;
            }
            ++it;
        }
    }

    void UIControl::SystemScreenSizeDidChanged(const Rect &newFullScreenRect)
    {
        ScreenSizeDidChanged(newFullScreenRect);

        List<UIControl*>::iterator it = childs.begin();
        while(it != childs.end())
        {
            isIteratorCorrupted = false;
            UIControl *current = *it;
            current->Retain();
            current->SystemScreenSizeDidChanged(newFullScreenRect);
            current->Release();
            if(isIteratorCorrupted)
            {
                it = childs.begin();
                continue;
            }
            ++it;
        }
    }




    void UIControl::WillAppear()
    {

    }
    void UIControl::WillDisappear()
    {

    }
    void UIControl::DidAppear()
    {

    }
    void UIControl::DidDisappear()
    {

    }
    void UIControl::ScreenSizeDidChanged(const Rect &newFullScreenRect)
    {

    }



    void UIControl::SystemUpdate(float32 timeElapsed)
    {
        UIControlSystem::Instance()->updateCounter++;
        Update(timeElapsed);
        isUpdated = true;
        List<UIControl*>::iterator it = childs.begin();
        for(; it != childs.end(); ++it)
        {
            (*it)->isUpdated = false;
        }

        it = childs.begin();
        while(it != childs.end())
        {
            isIteratorCorrupted = false;
            UIControl *current = *it;
            if(!current->isUpdated)
            {
                current->Retain();
                current->SystemUpdate(timeElapsed);
                current->Release();
                if(isIteratorCorrupted)
                {
                    it = childs.begin();
                    continue;
                }
            }
            ++it;
        }
    }

    void UIControl::SystemDraw(const UIGeometricData &geometricData)
    {
        if( !recursiveVisible )
            return;

        UIControlSystem::Instance()->drawCounter++;
        UIGeometricData drawData = GetLocalGeometricData();
        drawData.AddGeometricData(geometricData);

        const Color &parentColor = parent ? parent->GetBackground()->GetDrawColor() : Color::White;

        SetParentColor(parentColor);

        const Rect& unrotatedRect = drawData.GetUnrotatedRect();

        if(clipContents)
        {//WARNING: for now clip contents don't work for rotating controls if you have any ideas you are welcome
            RenderManager::Instance()->ClipPush();
            RenderManager::Instance()->ClipRect(unrotatedRect);
        }

        if(visible && visibleForUIEditor)
        {
            Draw(drawData);
        }

        isIteratorCorrupted = false;
        List<UIControl*>::iterator it = childs.begin();
        List<UIControl*>::iterator itEnd = childs.end();
        for(; it != itEnd; ++it)
        {
            (*it)->SystemDraw(drawData);
            DVASSERT(!isIteratorCorrupted);
        }

        if(visible && visibleForUIEditor)
        {
            DrawAfterChilds(drawData);
        }

        if(clipContents)
        {
            RenderManager::Instance()->ClipPop();
        }

        if(debugDrawEnabled)
        {
            RenderManager::Instance()->ClipPush();
            RenderManager::Instance()->RemoveClip();
            DrawDebugRect(drawData, false);
            DrawPivotPoint(unrotatedRect);
            RenderManager::Instance()->ClipPop();
        }
    }

    void UIControl::SetParentColor( const Color &parentColor )
    {
        GetBackground()->SetParentColor(parentColor);
    }

    void UIControl::DrawDebugRect(const UIGeometricData &gd, bool useAlpha)
    {
        Color oldColor = RenderManager::Instance()->GetColor();
        RenderManager::Instance()->ClipPush();

        if (useAlpha)
        {
            Color drawColor = debugDrawColor;
            drawColor.a = 0.4f;
            RenderManager::Instance()->SetColor(drawColor);
        }
        else
        {
            RenderManager::Instance()->SetColor(debugDrawColor);
        }

        if( gd.angle != 0.0f )
        {
            Polygon2 poly;
            gd.GetPolygon( poly );

            RenderHelper::Instance()->DrawPolygon( poly, true, RenderState::RENDERSTATE_2D_BLEND );
        }
        else
        {
            RenderHelper::Instance()->DrawRect( gd.GetUnrotatedRect(), RenderState::RENDERSTATE_2D_BLEND );
        }

        RenderManager::Instance()->ClipPop();
        RenderManager::Instance()->SetColor(oldColor);
    }

    void UIControl::DrawPivotPoint(const Rect &drawRect)
    {
        if (drawPivotPointMode == DRAW_NEVER)
        {
            return;
        }

        if (drawPivotPointMode == DRAW_ONLY_IF_NONZERO && GetPivotPoint().IsZero())
        {
            return;
        }

        static const float32 PIVOT_POINT_MARK_RADIUS = 10.0f;
        static const float32 PIVOT_POINT_MARK_HALF_LINE_LENGTH = 13.0f;

        Color oldColor = RenderManager::Instance()->GetColor();
        RenderManager::Instance()->ClipPush();
        RenderManager::Instance()->SetColor(Color(1.0f, 0.0f, 0.0f, 1.0f));

        Vector2 pivotPointCenter = drawRect.GetPosition() + GetPivotPoint();
        RenderHelper::Instance()->DrawCircle(pivotPointCenter, PIVOT_POINT_MARK_RADIUS, RenderState::RENDERSTATE_2D_BLEND);

        // Draw the cross mark.
        Vector2 lineStartPoint = pivotPointCenter;
        Vector2 lineEndPoint = pivotPointCenter;
        lineStartPoint.y -= PIVOT_POINT_MARK_HALF_LINE_LENGTH;
        lineEndPoint.y += PIVOT_POINT_MARK_HALF_LINE_LENGTH;
        RenderHelper::Instance()->DrawLine(lineStartPoint, lineEndPoint, RenderState::RENDERSTATE_2D_BLEND);

        lineStartPoint = pivotPointCenter;
        lineEndPoint = pivotPointCenter;
        lineStartPoint.x -= PIVOT_POINT_MARK_HALF_LINE_LENGTH;
        lineEndPoint.x += PIVOT_POINT_MARK_HALF_LINE_LENGTH;
        RenderHelper::Instance()->DrawLine(lineStartPoint, lineEndPoint, RenderState::RENDERSTATE_2D_BLEND);

        RenderManager::Instance()->ClipPop();
        RenderManager::Instance()->SetColor(oldColor);
    }

    bool UIControl::IsPointInside(const Vector2 &_point, bool expandWithFocus/* = false*/)
    {
        Vector2 point = _point;

        if(InputSystem::Instance()->IsCursorPining())
        {
            point.x = Core::Instance()->GetVirtualScreenWidth() / 2;
            point.y = Core::Instance()->GetVirtualScreenHeight() / 2;
        }

        const UIGeometricData &gd = GetGeometricData();
        Rect rect = gd.GetUnrotatedRect();
        if(expandWithFocus)
        {
            rect.dx += CONTROL_TOUCH_AREA*2;
            rect.dy += CONTROL_TOUCH_AREA*2;
            rect.x -= CONTROL_TOUCH_AREA;
            rect.y -= CONTROL_TOUCH_AREA;
        }
        if( gd.angle != 0 )
        {
            Vector2 testPoint;
            testPoint.x = (point.x - gd.position.x) * gd.cosA  + (gd.position.y - point.y) * -gd.sinA + gd.position.x;
            testPoint.y = (point.x - gd.position.x) * -gd.sinA  + (point.y - gd.position.y) * gd.cosA + gd.position.y;
            return rect.PointInside(testPoint);
        }

        return rect.PointInside(point);
    }

    bool UIControl::SystemProcessInput(UIEvent *currentInput)
    {
        if(!inputEnabled || !visible || controlState & STATE_DISABLED)
        {
            return false;
        }
        if(UIControlSystem::Instance()->GetExclusiveInputLocker()
           && UIControlSystem::Instance()->GetExclusiveInputLocker() != this)
        {
            return false;
        }

        switch (currentInput->phase)
        {
#if !defined(__DAVAENGINE_IPHONE__) && !defined(__DAVAENGINE_ANDROID__)
            case UIEvent::PHASE_KEYCHAR:
            {
                    Input(currentInput);
            }
            break;
            case UIEvent::PHASE_MOVE:
            {
                if (!currentInput->touchLocker && IsPointInside(currentInput->point))
                {
                    UIControlSystem::Instance()->SetHoveredControl(this);
                    Input(currentInput);
                    return true;
                }
            }
            break;
            case UIEvent::PHASE_WHEEL:
            {
                 Input(currentInput);
            }
            break;
#endif
            case UIEvent::PHASE_BEGAN:
            {
                if (!currentInput->touchLocker && IsPointInside(currentInput->point))
                {
                    if(multiInput || !currentInputID)
                    {

                        controlState |= STATE_PRESSED_INSIDE;
                        controlState &= ~STATE_NORMAL;
                        ++touchesInside;
                        ++totalTouches;
                        currentInput->controlState = UIEvent::CONTROL_STATE_INSIDE;

                        // Yuri Coder, 2013/12/18. Set the touch lockers before the EVENT_TOUCH_DOWN handler
                        // to have possibility disable control inside the EVENT_TOUCH_DOWN. See also DF-2943.
                        currentInput->touchLocker = this;
                        if(exclusiveInput)
                        {
                            UIControlSystem::Instance()->SetExclusiveInputLocker(this, currentInput->tid);
                        }

                        PerformEventWithData(EVENT_TOUCH_DOWN, currentInput);

                        if(!multiInput)
                        {
                            currentInputID = currentInput->tid;
                        }

                        Input(currentInput);
                        return true;
                    }
                    else
                    {
                        currentInput->touchLocker = this;
                        return true;
                    }

                }
            }
                break;
            case UIEvent::PHASE_DRAG:
            {
                if(currentInput->touchLocker == this)
                {
                    if(multiInput || currentInputID == currentInput->tid)
                    {
                        if(controlState & STATE_PRESSED_INSIDE || controlState & STATE_PRESSED_OUTSIDE)
                        {
                            if (IsPointInside(currentInput->point, true))
                            {
                                if(currentInput->controlState == UIEvent::CONTROL_STATE_OUTSIDE)
                                {
                                    currentInput->controlState = UIEvent::CONTROL_STATE_INSIDE;
                                    ++touchesInside;
                                    if(touchesInside > 0)
                                    {
                                        controlState |= STATE_PRESSED_INSIDE;
                                        controlState &= ~STATE_PRESSED_OUTSIDE;
#if !defined(__DAVAENGINE_IPHONE__) && !defined(__DAVAENGINE_ANDROID__)
                                        controlState |= STATE_HOVER;
#endif
                                    }
                                }
                            }
                            else
                            {
                                if(currentInput->controlState == UIEvent::CONTROL_STATE_INSIDE)
                                {
                                    currentInput->controlState = UIEvent::CONTROL_STATE_OUTSIDE;
                                    --touchesInside;
                                    if(touchesInside == 0)
                                    {
                                        controlState |= STATE_PRESSED_OUTSIDE;
                                        controlState &= ~STATE_PRESSED_INSIDE;
                                    }
                                }
                            }
                        }
                        Input(currentInput);
                    }
                    return true;
                }
            }
                break;
            case UIEvent::PHASE_ENDED:
            {
                if(currentInput->touchLocker == this)
                {
                    if(multiInput || currentInputID == currentInput->tid)
                    {
                        Input(currentInput);
                        if(currentInput->tid == currentInputID)
                        {
                            currentInputID = 0;
                        }
                        if(totalTouches > 0)
                        {
                            --totalTouches;
                            if(currentInput->controlState == UIEvent::CONTROL_STATE_INSIDE)
                            {
                                --touchesInside;
#if !defined(__DAVAENGINE_IPHONE__) && !defined(__DAVAENGINE_ANDROID__)
                                if(totalTouches == 0)
                                {
                                    controlState |= STATE_HOVER;
                                }
#endif
                            }

                            currentInput->controlState = UIEvent::CONTROL_STATE_RELEASED;

                            if(totalTouches == 0)
                            {
                                if (IsPointInside(currentInput->point, true))
                                {
                                    if (UIControlSystem::Instance()->GetFocusedControl() != this && focusEnabled)
                                    {
                                        UIControlSystem::Instance()->SetFocusedControl(this, false);
                                    }
                                    PerformEventWithData(EVENT_TOUCH_UP_INSIDE, currentInput);
                                }
                                else
                                {
                                    PerformEventWithData(EVENT_TOUCH_UP_OUTSIDE, currentInput);
                                }
                                controlState &= ~STATE_PRESSED_INSIDE;
                                controlState &= ~STATE_PRESSED_OUTSIDE;
                                controlState |= STATE_NORMAL;
                                if(UIControlSystem::Instance()->GetExclusiveInputLocker() == this)
                                {
                                    UIControlSystem::Instance()->SetExclusiveInputLocker(NULL, -1);
                                }
                            }
                            else if(touchesInside <= 0)
                            {
                                controlState |= STATE_PRESSED_OUTSIDE;
                                controlState &= ~STATE_PRESSED_INSIDE;
#if !defined(__DAVAENGINE_IPHONE__) && !defined(__DAVAENGINE_ANDROID__)
                                controlState &= ~STATE_HOVER;
#endif
                            }
                        }
                    }

                    currentInput->touchLocker = NULL;
                    return true;
                }
            }
                break;
            case UIEvent::PHASE_JOYSTICK:
            {
                Input(currentInput);
            }
        }

        return false;
    }

    bool UIControl::SystemInput(UIEvent *currentInput)
    {
        UIControlSystem::Instance()->inputCounter++;
        isUpdated = true;

        if( !recursiveVisible )
            return false;

        //if(currentInput->touchLocker != this)
        {
            if(clipContents
               && (currentInput->phase != UIEvent::PHASE_DRAG
                   && currentInput->phase != UIEvent::PHASE_ENDED
                   && currentInput->phase != UIEvent::PHASE_KEYCHAR
                   && currentInput->phase != UIEvent::PHASE_JOYSTICK))
            {
                if(!IsPointInside(currentInput->point))
                {
                    return false;
                }
            }

            List<UIControl*>::reverse_iterator it = childs.rbegin();
            List<UIControl*>::reverse_iterator itEnd = childs.rend();
            for(; it != itEnd; ++it)
            {
                (*it)->isUpdated = false;
            }

            it = childs.rbegin();
            itEnd = childs.rend();
            while(it != itEnd)
            {
                isIteratorCorrupted = false;
                UIControl *current = *it;
                if(!current->isUpdated)
                {
                    current->Retain();
                    if(current->inputProcessorsCount > 0 && current->SystemInput(currentInput))
                    {
                        current->Release();
                        return true;
                    }
                    current->Release();
                    if(isIteratorCorrupted)
                    {
                        it = childs.rbegin();
                        continue;
                    }
                }
                ++it;
            }
        }
        return SystemProcessInput(currentInput);
    }

    void UIControl::SystemInputCancelled(UIEvent *currentInput)
    {
        if(currentInput->controlState != UIEvent::CONTROL_STATE_RELEASED)
        {
            --totalTouches;
        }
        if(currentInput->controlState == UIEvent::CONTROL_STATE_INSIDE)
        {
            --touchesInside;
        }

        if(touchesInside == 0)
        {
            controlState &= ~STATE_PRESSED_INSIDE;
            controlState &= ~STATE_PRESSED_OUTSIDE;
            controlState |= STATE_NORMAL;
            if(UIControlSystem::Instance()->GetExclusiveInputLocker() == this)
            {
                UIControlSystem::Instance()->SetExclusiveInputLocker(NULL, -1);
            }
        }

        currentInput->controlState = UIEvent::CONTROL_STATE_RELEASED;
        if(currentInput->tid == currentInputID)
        {
            currentInputID = 0;
        }
        currentInput->touchLocker = NULL;


        InputCancelled(currentInput);
    }

    void UIControl::SystemDidSetHovered()
    {
        controlState |= STATE_HOVER;
        PerformEventWithData(EVENT_HOVERED_SET, NULL);
        DidSetHovered();
    }

    void UIControl::SystemDidRemoveHovered()
    {
        PerformEventWithData(EVENT_HOVERED_REMOVED, NULL);
        controlState &= ~STATE_HOVER;
        DidRemoveHovered();
    }

    void UIControl::DidSetHovered()
    {
    }

    void UIControl::DidRemoveHovered()
    {
    }

    void UIControl::Input(UIEvent *currentInput)
    {
        currentInput->SetInputHandledType(UIEvent::INPUT_NOT_HANDLED);
    }

    void UIControl::InputCancelled(UIEvent *currentInput)
    {
    }

    void UIControl::Update(float32 timeElapsed)
    {

    }
    void UIControl::Draw(const UIGeometricData &geometricData)
    {
        background->Draw(geometricData);
    }
    void UIControl::DrawAfterChilds(const UIGeometricData &geometricData)
    {

    }

    void UIControl::SystemWillBecomeVisible()
    {
        WillBecomeVisible();

        List<UIControl*>::const_iterator it = childs.begin();
        List<UIControl*>::const_iterator end = childs.end();
        for (; it != end; ++it)
        {
            if ((*it)->GetRecursiveVisible())
                (*it)->SystemWillBecomeVisible();
        }
    }

    void UIControl::SystemWillBecomeInvisible()
    {
        if (GetHover())
        {
            UIControlSystem::Instance()->SetHoveredControl(NULL);
        }
        if (UIControlSystem::Instance()->GetFocusedControl() == this)
        {
            UIControlSystem::Instance()->SetFocusedControl(NULL, true);
        }
        if (GetInputEnabled())
        {
            UIControlSystem::Instance()->CancelInputs(this, false);
        }

        List<UIControl*>::const_iterator it = childs.begin();
        List<UIControl*>::const_iterator end = childs.end();
        for (; it != end; ++it)
        {
            if ((*it)->GetRecursiveVisible())
                (*it)->SystemWillBecomeInvisible();
        }

        WillBecomeInvisible();
    }

    void UIControl::WillBecomeVisible()
    {
    }

    void UIControl::WillBecomeInvisible()
    {
    }

    bool UIControl::SavePropertiesToYamlNode(YamlNode *node, UIControl *baseControl, const UIYamlLoader *loader)
    {
        if (node->GetType() != YamlNode::TYPE_MAP)
        {
            DVASSERT(false);
            return false;
        }
        // Transform data
        // Position
        const Vector2 &position = GetPosition();
        if (baseControl->GetPosition() != position)
        {
            node->Set("position", position);
        }
        // Size
        const Vector2 &size = GetSize();
        if (baseControl->GetSize() != size)
        {
            node->Set("size", size);
        }
        // Pivot
        if (baseControl->GetPivotPoint() != GetPivotPoint())
        {
            node->Set("pivot", GetPivotPoint());
        }
        // Angle
        if (baseControl->GetAngle() != GetAngle())
        {
            node->Set("angle", GetAngle());
        }
        // Transform data

        // Recursive Visible
        if (baseControl->GetRecursiveVisible() != GetRecursiveVisible())
        {
            node->Set("recursiveVisible", GetRecursiveVisible());
        }
        // Visible
        if (baseControl->GetVisible() != GetVisible())
        {
            node->Set("visible", GetVisible());
        }
        // Enabled
        if (baseControl->GetDisabled() != GetDisabled())
        {
            node->Set("enabled", !GetDisabled());
        }
        // Clip contents
        if (baseControl->GetClipContents() != GetClipContents())
        {
            node->Set("clip", GetClipContents());
        }
        // Input
        if (baseControl->GetInputEnabled() != GetInputEnabled())
        {
            node->Set("noInput", !GetInputEnabled());
        }
        // Tag
        if (baseControl->GetTag() != GetTag())
        {
            node->Set("tag", GetTag());
        }
        // Initial state.
        if (baseControl->GetInitialState() != GetInitialState())
        {
            node->Set("initialState", GetInitialState());
        }

        // Anchor data
        // Left Align
        if (GetLeftAlignEnabled())
        {
            node->Set("leftAlign", GetLeftAlign());
        }
        // Horizontal Center Align
        if (GetHCenterAlignEnabled())
        {
            node->Set("hcenterAlign", GetHCenterAlign());
        }
        // Right Align
        if (GetRightAlignEnabled())
        {
            node->Set("rightAlign", GetRightAlign());
        }
        // Top Align
        if (GetTopAlignEnabled())
        {
            node->Set("topAlign", GetTopAlign());
        }
        // Vertical Center Align
        if (GetVCenterAlignEnabled())
        {
            node->Set("vcenterAlign", GetVCenterAlign());
        }
        // Bottom Align
        if (GetBottomAlignEnabled())
        {
            node->Set("bottomAlign", GetBottomAlign());
        }
        // Anchor data

        UIControlBackground *baseBackground = baseControl->GetBackground();
        if (!baseBackground->IsEqualTo(GetBackground()))
        {
            // Draw type, obligatory for UI controls.
            UIControlBackground::eDrawType drawType =  GetBackground()->GetDrawType();
            if (baseBackground->GetDrawType() != drawType)
            {
                node->Set("drawType", loader->GetDrawTypeNodeValue(drawType));
            }
            // Sprite
            String spritePath = Sprite::GetPathString(GetBackground()->GetSprite());
            if (Sprite::GetPathString(baseBackground->GetSprite()) != spritePath)
            {
                node->Set("sprite", spritePath);
            }
            // Frame
            if (baseBackground->GetFrame() != GetBackground()->GetFrame())
            {
                node->Set("frame", GetFrame());
            }
            // Color
            const Color &color =  GetBackground()->GetColor();
            if (baseBackground->GetColor() != color)
            {
                node->Set("color", VariantType(color));
            }
            // Color inherit
            UIControlBackground::eColorInheritType colorInheritType =  GetBackground()->GetColorInheritType();
            if (baseBackground->GetColorInheritType() != colorInheritType)
            {
                node->Set("colorInherit", loader->GetColorInheritTypeNodeValue(colorInheritType));
            }
            // Per pixel accuracy
            UIControlBackground::ePerPixelAccuracyType perPixelAccuracyType = GetBackground()->GetPerPixelAccuracyType();
            if (baseBackground->GetPerPixelAccuracyType() != perPixelAccuracyType)
            {
                node->Set("perPixelAccuracy", loader->GetPerPixelAccuracyTypeNodeValue(perPixelAccuracyType));
            }
            // Align
            int32 align = GetBackground()->GetAlign();
            if (baseBackground->GetAlign() != align)
            {
                node->AddNodeToMap("align", loader->GetAlignNodeValue(align));
            }
            // LeftRightStretchCapNode
            if (baseBackground->GetLeftRightStretchCap() != GetBackground()->GetLeftRightStretchCap())
            {
                node->Set("leftRightStretchCap", GetBackground()->GetLeftRightStretchCap());
            }
            // topBottomStretchCap
            if (baseBackground->GetTopBottomStretchCap() != GetBackground()->GetTopBottomStretchCap())
            {
                node->Set("topBottomStretchCap", GetBackground()->GetTopBottomStretchCap());
            }
            // spriteModification
            if (baseBackground->GetModification() != GetBackground()->GetModification())
            {
                node->Set("spriteModification", GetBackground()->GetModification());
            }

            // margins.
            const UIControlBackground::UIMargins* margins = GetBackground()->GetMargins();
            if (margins)
            {
                node->Set("margins", margins->AsVector4());
            }
        }
        return true;
    }

    bool UIControl::LoadPropertiesFromYamlNode( const YamlNode *node, UIYamlLoader *loader )
    {
        if (node->GetType() != YamlNode::TYPE_MAP)
        {
            DVASSERT(false);
            return false;
        }

        const YamlNode * rectNode = node->Get("rect");
        if (rectNode)
        {
            Rect rect = rectNode->AsRect();
            SetRect(rect);
        }
        else
        {
            const YamlNode * positionNode = node->Get("position");
            if (positionNode)
                SetPosition(positionNode->AsVector2());

            const YamlNode * sizeNode = node->Get("size");
            if (sizeNode)
                SetSize(sizeNode->AsVector2());
        }

        const YamlNode * pivotNode = node->Get("pivot");
        if (pivotNode)
        {
            DVASSERT(pivotNode->GetType() == YamlNode::TYPE_ARRAY);
            SetPivotPoint(pivotNode->AsPoint());
        }

        const YamlNode * angleNode = node->Get("angle");
        if (angleNode)
        {
            SetAngle(angleNode->AsFloat());
        }

        const YamlNode * recursiveVisibleNode = node->Get("recursiveVisible");
        if (recursiveVisibleNode)
        {
            SetRecursiveVisible(recursiveVisibleNode->AsBool());
        }

        const YamlNode * visibleNode = node->Get("visible");
        if (visibleNode)
        {
            SetVisible(visibleNode->AsBool());
        }

        const YamlNode * enabledNode = node->Get("enabled");
        if (enabledNode)
        {
            SetDisabled(!enabledNode->AsBool());
        }

        const YamlNode * clipNode = node->Get("clip");
        if (clipNode)
        {
            SetClipContents(clipNode->AsBool());
        }

        const YamlNode * inputNode = node->Get("noInput");
        if (inputNode)
        {
            SetInputEnabled(!inputNode->AsBool(), false);
        }

        const YamlNode * tagNode = node->Get("tag");
        if (tagNode)
        {
            SetTag(tagNode->AsInt32());
        }

        const YamlNode * initialStateNode = node->Get("initialState");
        if (initialStateNode)
        {
            int32 newInitialState = initialStateNode->AsInt32();
            SetInitialState(newInitialState);
            SetState(newInitialState);
        }

        const YamlNode * leftAlignNode = node->Get("leftAlign");
        if (leftAlignNode)
        {
            int32 leftAlign = leftAlignNode->AsInt32();
            SetLeftAlignEnabled(true);
            SetLeftAlign(leftAlign);
        }

        const YamlNode * hcenterAlignNode = node->Get("hcenterAlign");
        if (hcenterAlignNode)
        {
            int32 hcenterAlign = hcenterAlignNode->AsInt32();
            SetHCenterAlignEnabled(true);
            SetHCenterAlign(hcenterAlign);
        }

        const YamlNode * rightAlignNode = node->Get("rightAlign");
        if (rightAlignNode)
        {
            int32 rightAlign = rightAlignNode->AsInt32();
            SetRightAlignEnabled(true);
            SetRightAlign(rightAlign);
        }

        const YamlNode * topAlignNode = node->Get("topAlign");
        if (topAlignNode)
        {
            int32 topAlign = topAlignNode->AsInt32();
            SetTopAlignEnabled(true);
            SetTopAlign(topAlign);
        }

        const YamlNode * vcenterAlignNode = node->Get("vcenterAlign");
        if (vcenterAlignNode)
        {
            int32 vcenterAlign = vcenterAlignNode->AsInt32();
            SetVCenterAlignEnabled(true);
            SetVCenterAlign(vcenterAlign);
        }

        const YamlNode * bottomAlignNode = node->Get("bottomAlign");
        if (bottomAlignNode)
        {
            int32 bottomAlign = bottomAlignNode->AsInt32();
            SetBottomAlignEnabled(true);
            SetBottomAlign(bottomAlign);
        }

        const YamlNode * drawTypeNode = node->Get("drawType");
        if (drawTypeNode)
        {
            GetBackground()->SetDrawType((UIControlBackground::eDrawType)loader->GetDrawTypeFromNode(drawTypeNode));
        }

        const YamlNode * spriteNode = node->Get("sprite");
        if (spriteNode)
        {
            GetBackground()->SetSprite(spriteNode->AsString(), GetFrame());
        }

        const YamlNode * frameNode = node->Get("frame");
        if (frameNode)
        {
            GetBackground()->SetFrame(frameNode->AsInt32());
        }

        const YamlNode * colorNode = node->Get("color");
        if (colorNode)
        {
            GetBackground()->SetColor(loader->GetColorFromYamlNode(colorNode));
        }

        const YamlNode * perPixelAccuracyTypeNode = node->Get("perPixelAccuracy");
        if(perPixelAccuracyTypeNode)
        {
            GetBackground()->SetPerPixelAccuracyType((UIControlBackground::ePerPixelAccuracyType)loader->GetPerPixelAccuracyTypeFromNode(perPixelAccuracyTypeNode));
        }

        const YamlNode * colorInheritNode = node->Get("colorInherit");
        if (colorInheritNode)
        {
            GetBackground()->SetColorInheritType((UIControlBackground::eColorInheritType)loader->GetColorInheritTypeFromNode(colorInheritNode));
        }

        const YamlNode * alignNode = node->Get("align");
        if (alignNode)
        {
            GetBackground()->SetAlign(loader->GetAlignFromYamlNode(alignNode));
        }

        const YamlNode * leftRightStretchCapNode = node->Get("leftRightStretchCap");
        if (leftRightStretchCapNode)
        {
            GetBackground()->SetLeftRightStretchCap(leftRightStretchCapNode->AsFloat());
        }

        const YamlNode * topBottomStretchCapNode = node->Get("topBottomStretchCap");
        if (topBottomStretchCapNode)
        {
            GetBackground()->SetTopBottomStretchCap(topBottomStretchCapNode->AsFloat());
        }

        const YamlNode * spriteModificationNode = node->Get("spriteModification");
        if (spriteModificationNode)
        {
            GetBackground()->SetModification(spriteModificationNode->AsInt32());
        }
        
        const YamlNode * marginsNode = node->Get("margins");
        if (marginsNode)
        {
            UIControlBackground::UIMargins margins(marginsNode->AsVector4());
            GetBackground()->SetMargins(&margins);
        }

        return true;
    }

    Animation * UIControl::WaitAnimation(float32 time, int32 track)
    {
        Animation * animation = new Animation(this, time, Interpolation::LINEAR);
        animation->Start(track);
        return animation;
    }

    Animation * UIControl::PositionAnimation(const Vector2 & _position, float32 time, Interpolation::FuncType interpolationFunc, int32 track)
    {
        LinearAnimation<Vector2> * animation = new LinearAnimation<Vector2>(this, &relativePosition, _position, time, interpolationFunc);
        animation->Start(track);
        return animation;
    }

    Animation * UIControl::SizeAnimation(const Vector2 & _size, float32 time, Interpolation::FuncType interpolationFunc, int32 track)
    {
        LinearAnimation<Vector2> * animation = new LinearAnimation<Vector2>(this, &size, _size, time, interpolationFunc);
        animation->Start(track);
        return animation;
    }

    Animation * UIControl::ScaleAnimation(const Vector2 & newScale, float32 time, Interpolation::FuncType interpolationFunc, int32 track)
    {
        LinearAnimation<Vector2> * animation = new LinearAnimation<Vector2>(this, &scale, newScale, time, interpolationFunc);
        animation->Start(track);
        return animation;
    }

    Animation * UIControl::AngleAnimation(float32 newAngle, float32 time, Interpolation::FuncType interpolationFunc /*= Interpolation::LINEAR*/, int32 track /*= 0*/)
    {
        LinearAnimation<float32> * animation = new LinearAnimation<float32>(this, &angle, newAngle, time, interpolationFunc);
        animation->Start(track);
        return animation;
    }


    Animation * UIControl::MoveAnimation(const Rect & rect, float time, Interpolation::FuncType interpolationFunc, int32 track)
    {
        TwoVector2LinearAnimation *animation = new TwoVector2LinearAnimation(this
                , &relativePosition, Vector2(rect.x + pivotPoint.x, rect.y + pivotPoint.y)
                , &size, Vector2(rect.dx, rect.dy), time, interpolationFunc);
        animation->Start(track);
        return animation;
    }

    Animation * UIControl::ScaledRectAnimation(const Rect & rect, float32 time, Interpolation::FuncType interpolationFunc, int32 track)
    {
        Vector2 finalScale(rect.dx / size.x, rect.dy / size.y);

        TwoVector2LinearAnimation *animation = new TwoVector2LinearAnimation(this
                , &relativePosition, Vector2(rect.x + pivotPoint.x * finalScale.x, rect.y + pivotPoint.y * finalScale.y)
                , &scale, finalScale, time, interpolationFunc);
        animation->Start(track);
        return animation;
    }

    Animation * UIControl::ScaledSizeAnimation(const Vector2 & newSize, float32 time, Interpolation::FuncType interpolationFunc, int32 track)
    {
        Vector2 finalScale(newSize.x / size.x, newSize.y / size.y);
        LinearAnimation<Vector2> * animation = new LinearAnimation<Vector2>(this, &scale, finalScale, time, interpolationFunc);
        animation->Start(track);
        return animation;
    }


    void UIControl::TouchableAnimationCallback(BaseObject * caller, void * param, void *callerData)
    {
        bool * params = (bool*)param;
        SetInputEnabled(params[0], params[1]);
        delete[]params;
    }

    Animation * UIControl::TouchableAnimation(bool touchable, bool hierarhic/* = true*/, int32 track/* = 0*/)
    {
        //TODO: change to bool animation - Dizz
        Animation * animation = new Animation(this, 0.01f, Interpolation::LINEAR);
        bool * params = new bool[2];
        params[0] = touchable;
        params[1] = hierarhic;
        animation->AddEvent(Animation::EVENT_ANIMATION_START, Message(this, &UIControl::TouchableAnimationCallback, (void*)params));
        animation->Start(track);
        return animation;
    }

    void UIControl::DisabledAnimationCallback(BaseObject * caller, void * param, void *callerData)
    {
        bool * params = (bool*)param;
        SetDisabled(params[0], params[1]);
        delete[]params;
    }

    Animation * UIControl::DisabledAnimation(bool disabled, bool hierarhic/* = true*/, int32 track/* = 0*/)
    {
        //TODO: change to bool animation - Dizz
        Animation * animation = new Animation(this, 0.01f, Interpolation::LINEAR);
        bool * params = new bool[2];
        params[0] = disabled;
        params[1] = hierarhic;
        animation->AddEvent(Animation::EVENT_ANIMATION_START, Message(this, &UIControl::DisabledAnimationCallback, (void*)params));
        animation->Start(track);
        return animation;
    }

    void UIControl::VisibleAnimationCallback(BaseObject * caller, void * param, void *callerData)
    {
        bool * params = (bool*)param;
        SetVisible(params[0], params[1]);
        delete[]params;
    }

    void UIControl::RecursiveVisibleAnimationCallback( BaseObject * caller, void * param, void *callerData )
    {
        bool visible = ( pointer_size(param) > 0 );
        SetRecursiveVisible(visible);
    }

    Animation * UIControl::VisibleAnimation(bool visible, bool hierarhic/* = true*/, int32 track/* = 0*/)
    {
        //TODO: change to bool animation - Dizz
        Animation * animation = new Animation(this, 0.01f, Interpolation::LINEAR);
        bool * params = new bool[2];
        params[0] = visible;
        params[1] = hierarhic;
        animation->AddEvent(Animation::EVENT_ANIMATION_START, Message(this, &UIControl::VisibleAnimationCallback, (void*)params));
        animation->Start(track);
        return animation;
    }

    Animation * UIControl::RecursiveVisibleAnimation(bool visible, int32 track/* = 0*/)
    {
        Animation * animation = new Animation(this, 0.01f, Interpolation::LINEAR);
        animation->AddEvent(Animation::EVENT_ANIMATION_START, Message(this, &UIControl::RecursiveVisibleAnimationCallback, (void*)(pointer_size)visible));
        animation->Start(track);
        return animation;
    }

    void UIControl::RemoveControlAnimationCallback(BaseObject * caller, void * param, void *callerData)
    {
        if(parent)
        {
            parent->RemoveControl(this);
        }
    }

    Animation * UIControl::RemoveControlAnimation(int32 track)
    {
        Animation * animation = new Animation(this, 0.01f, Interpolation::LINEAR);
        animation->AddEvent(Animation::EVENT_ANIMATION_START, Message(this, &UIControl::RemoveControlAnimationCallback));
        animation->Start(track);
        return animation;
    }

    Animation * UIControl::ColorAnimation(const Color & finalColor, float32 time, Interpolation::FuncType interpolationFunc, int32 track)
    {
        LinearAnimation<Color> * animation = new LinearAnimation<Color>(this, &background->color, finalColor, time, interpolationFunc);
        animation->Start(track);
        return animation;
    }

    void UIControl::SetDebugDraw(bool _debugDrawEnabled, bool hierarchic/* = false*/)
    {
        debugDrawEnabled = _debugDrawEnabled;
        if(hierarchic)
        {
            List<UIControl*>::iterator it = childs.begin();
            for(; it != childs.end(); ++it)
            {
                (*it)->SetDebugDraw(debugDrawEnabled, hierarchic);
            }
        }
    }

    void UIControl::SetDebugDrawColor(const Color& color)
    {
        debugDrawColor = color;
    }

    const Color &UIControl::GetDebugDrawColor() const
    {
        return debugDrawColor;
    }

    void UIControl::SetDrawPivotPointMode(eDebugDrawPivotMode mode, bool hierarchic /*=false*/)
    {
        drawPivotPointMode = mode;
        if (hierarchic)
        {
            List<UIControl*>::iterator it = childs.begin();
            for(; it != childs.end(); ++it)
            {
                (*it)->SetDrawPivotPointMode(mode, hierarchic);
            }
        }
    }

    bool UIControl::IsLostFocusAllowed( UIControl *newFocus )
    {
        return true;
    }

    void UIControl::SystemOnFocusLost(UIControl *newFocus)
    {
        PerformEvent(EVENT_FOCUS_LOST);
        OnFocusLost(newFocus);
    }

    void UIControl::SystemOnFocused()
    {
        PerformEvent(EVENT_FOCUS_SET);
        OnFocused();
    }

    void UIControl::OnFocusLost(UIControl *newFocus)
    {
    }

    void UIControl::OnFocused()
    {
    }

    void UIControl::SetSizeFromBg(bool pivotToCenter)
    {
        SetSize(GetBackground()->GetSprite()->GetSize());

        if (pivotToCenter)
        {
            SetPivot(Vector2(0.5f, 0.5f));
        }
    }

    void UIControl::RecalculateAlignProperties()
    {
        Rect controlRect = this->GetRect(false);

        // Set left align property
        if (this->GetLeftAlignEnabled())
        {
            leftAlign = (int32)controlRect.x;
        }
        // Set top align property
        if (this->GetTopAlignEnabled())
        {
            topAlign = (int32)controlRect.y;
        }

        UIControl *parent = this->GetParent();
        if (!parent)
            return;

        // Set right align property
        if (this->GetRightAlignEnabled())
        {
            rightAlign = (int32)(parent->GetSize().x - controlRect.x - controlRect.dx);
        }
        // Set hcenter align property
        if (this->GetHCenterAlignEnabled())
        {
            if (this->GetLeftAlignEnabled())
            {
                hcenterAlign = (int32)(controlRect.x + controlRect.dx - Round(parent->GetSize().x / 2));
            }
            else if (this->GetRightAlignEnabled())
            {
                hcenterAlign = (int32)(controlRect.x - Round(parent->GetSize().x / 2));
            }
            else
            {
                hcenterAlign = (int32)(controlRect.x - Round(parent->GetSize().x / 2) + Round(controlRect.dx / 2));
            }
        }
        // Set bottom align property
        if (this->GetBottomAlignEnabled())
        {
            bottomAlign = (int32)(parent->GetSize().y - controlRect.y - controlRect.dy);
        }
        // Set Vcenter align property
        if (this->GetVCenterAlignEnabled())
        {
            if (this->GetTopAlignEnabled())
            {
                vcenterAlign = (int32)(controlRect.y + controlRect.dy - Round(parent->GetSize().y / 2));
            }
            else if (this->GetBottomAlignEnabled())
            {
                vcenterAlign = (int32)(controlRect.y - Round(parent->GetSize().y / 2));
            }
            else
            {
                vcenterAlign = (int32)(controlRect.y - Round(parent->GetSize().y / 2) + Round(controlRect.dy / 2));
            }
        }
    }

    void UIControl::RecalculateChildsSize()
    {
        const List<UIControl*>& realChildren = this->GetChildren();//YZ recalculate size for all controls
        for(List<UIControl*>::const_iterator iter = realChildren.begin(); iter != realChildren.end(); ++iter)
        {
            UIControl* child = (*iter);
            if (child)
            {
                // Recalculate horizontal aligns
                if (child->GetHCenterAlignEnabled())
                {
                    int32 hcenterAlign = child->GetHCenterAlign();
                    child->SetHCenterAlign(hcenterAlign);
                }
                else if (child->GetRightAlignEnabled())
                {
                    int32 rightAlign = child->GetRightAlign();
                    child->SetRightAlign(rightAlign);
                }
                // Recalculate vertical aligns
                if (child->GetVCenterAlignEnabled())
                {
                    int32 vcenterAlign = child->GetVCenterAlign();
                    child->SetVCenterAlign(vcenterAlign);
                }
                else if (child->GetBottomAlignEnabled())
                {
                    int32 bottomAlign = child->GetBottomAlign();
                    child->SetBottomAlign(bottomAlign);
                }
            }
        }
    }

    float32 UIControl::GetSizeX(UIControl *parent, int32 leftAlign, int32 rightAlign, bool useHalfParentSize /* = FALSE*/)
    {
        if (!parent)
            return 0;

        float32 parentSize = useHalfParentSize ? Round(parent->GetSize().x / 2) : parent->GetSize().x;
        float32 size = abs(parentSize - leftAlign - rightAlign);

        return size;
    }

    float32 UIControl::GetSizeY(UIControl *parent, int32 topAlign, int32 bottomAlign, bool useHalfParentSize /* = FALSE*/)
    {
        if (!parent)
            return 0;

        float32 parentSize = useHalfParentSize ? Round(parent->GetSize().y / 2) : parent->GetSize().y;
        float32 size = abs(parentSize - topAlign - bottomAlign);

        return size;
    }

    float32 UIControl::GetCenterX(UIControl *parent, int32 centerAlign, UIControl* child)
    {
        if (!parent || !child)
            return 0;

        float32 position = Round(parent->GetSize().x / 2) + centerAlign - Round(child->GetSize().x / 2);

        return position;
    }

    float32 UIControl::GetCenterY(UIControl *parent, int32 centerAlign, UIControl* child)
    {
        if (!parent || !child)
            return 0;

        float32 position = Round(parent->GetSize().y / 2) + centerAlign - Round(child->GetSize().y / 2);

        return position;
    }

    float32 UIControl::GetRelativeX(UIControl *parent, int32 align)
    {
        if (!parent)
            return (float32)align;

        float32 position = Round(parent->GetSize().x / 2) + align;

        return position;
    }

    float32 UIControl::GetRelativeX(UIControl *parent, int32 align, UIControl *child, bool useHalfParentSize /* = FALSE*/)
    {
        if (!parent || !child)
            return (float32)align;

        float32 parentSize = useHalfParentSize ? Round(parent->GetSize().x / 2) : parent->GetSize().x;
        float32 position = parentSize - align - child->GetSize().x;

        return position;
    }

    float32 UIControl::GetRelativeY(UIControl *parent, int32 align)
    {
        if (!parent)
            return (float32)align;

        float32 position = Round(parent->GetSize().y / 2) + align;

        return position;
    }

    float32 UIControl::GetRelativeY(UIControl *parent, int32 align, UIControl *child, bool useHalfParentSize /* = FALSE*/)
    {
        if (!parent || !child)
            return (float32)align;

        float32 parentSize = useHalfParentSize ? Round(parent->GetSize().y / 2) : parent->GetSize().y;
        float32 position = parentSize - align - child->GetSize().y;

        return position;
    }

    void UIControl::RecalculatePivotPoint(const Rect &newRect)
    {
        Rect oldRect = this->GetRect();
        // DF-2009 - Change proportianal pivot point accodring to new size
        if ((oldRect.dx > 0 && oldRect.dy > 0))
        {
            float32 xMultiup = pivotPoint.x / oldRect.dx;
            float32 yMultiup = pivotPoint.y / oldRect.dy;

            pivotPoint.x = xMultiup * newRect.dx;
            pivotPoint.y = yMultiup * newRect.dy;
        }
    }

    void UIControl::UpdateLayout()
    {
        RecalculateChildsSize();
    }

    void UIControl::ApplyAlignSettingsForChildren()
    {
        UpdateLayout();
    }

    const String & UIControl::GetControlClassName() const
    {
        return GetClassName();
    }

    const String &UIControl::GetCustomControlClassName() const
    {
        return customControlType;
    }

    void UIControl::SetCustomControlClassName(const String& value)
    {
        customControlType = value;
    }

    void UIControl::ResetCustomControlClassName()
    {
        customControlType = String();
    }

    int32 UIControl::GetInitialState() const
    {
        return initialState;
    }

    void UIControl::SetInitialState(int32 newState)
    {
        initialState = newState;
    }

    void UIControl::RegisterInputProcessor()
    {
        inputProcessorsCount++;
        if (parent)
        {
            parent->RegisterInputProcessor();
        }
    }

    void UIControl::RegisterInputProcessors(int32 processorsCount)
    {
        inputProcessorsCount += processorsCount;
        if (parent)
        {
            parent->RegisterInputProcessors(processorsCount);
        }
    }

    void UIControl::UnregisterInputProcessor()
    {
        inputProcessorsCount--;
        DVASSERT(inputProcessorsCount >= 0);
        if (parent)
        {
            parent->UnregisterInputProcessor();
        }
    }
    void UIControl::UnregisterInputProcessors(int32 processorsCount)
    {
        inputProcessorsCount -= processorsCount;
        DVASSERT(inputProcessorsCount >= 0);
        if (parent)
        {
            parent->UnregisterInputProcessors(processorsCount);
        }
    }

    void UIControl::DumpInputs(int32 depthLevel)
    {
        String outStr;
        for (int32 i = 0; i < depthLevel; i++)
        {
            outStr += "| ";
        }
        outStr += "\\-";
        outStr += name;
        if (inputProcessorsCount > 0)
        {
            outStr += " ";
            outStr += Format("%d", inputProcessorsCount);
        }

        if (inputEnabled)
        {
            outStr += " ***";
        }
        Logger::Info("%s", outStr.c_str());
        List<UIControl*>::iterator it = childs.begin();
        for(; it != childs.end(); ++it)
        {
            (*it)->DumpInputs(depthLevel + 1);
        }
    }    

    BaseObject *UIControl::GetCustomData() const
    {
        return customData;
    }
    
    void UIControl::SetCustomData(BaseObject *data)
    {
        if (data != customData)
        {
            SafeRelease(customData);
            customData = SafeRetain(data);
        }
    }
    

    int32 UIControl::GetBackgroundComponentsCount() const
    {
        return 1;
    }
    
    UIControlBackground *UIControl::GetBackgroundComponent(int32 index) const
    {
        DVASSERT(index == 0);
        return background;
    }
    
    UIControlBackground *UIControl::CreateBackgroundComponent(int32 index) const
    {
        DVASSERT(index == 0);
        return new UIControlBackground();
    }
    
    void UIControl::SetBackgroundComponent(int32 index, UIControlBackground *bg)
    {
        DVASSERT(index == 0);
        SetBackground(bg);
    }
    
    String UIControl::GetBackgroundComponentName(int32 index) const
    {
        DVASSERT(index == 0);
        return "Background";
    }
    
    int32 UIControl::GetInternalControlsCount() const
    {
        return 0;
    }
    
    UIControl *UIControl::GetInternalControl(int32 index) const
    {
        DVASSERT(false);
        return NULL;
    }
    
    UIControl *UIControl::CreateInternalControl(int32 index) const
    {
        DVASSERT(false);
        return NULL;
    }
    
    void UIControl::SetInternalControl(int32 index, UIControl *control)
    {
        DVASSERT(false);
    }

    String UIControl::GetInternalControlName(int32 index) const
    {
        DVASSERT(false);
        return "";
    }

    String UIControl::GetInternalControlDescriptions() const
    {
        DVASSERT(false);
        return "";
    }

}
