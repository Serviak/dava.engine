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


#ifndef __DAVAENGINE_UI_CONTROL_H__
#define __DAVAENGINE_UI_CONTROL_H__

#include "Base/BaseTypes.h"
#include "UI/UIControlBackground.h"
#include "Animation/AnimatedObject.h"
#include "Animation/Interpolation.h"

namespace DAVA
{
class UIYamlLoader;
class Animation;
class EventDispatcher;
class UIEvent;
class UIControlBackground;
class Message;

#define CONTROL_TOUCH_AREA  15
    /**
     \ingroup controlsystem
     \brief Compound of geometric transformations used to draw control in the screen space.
     */

class UIGeometricData
{
    friend class UIControl;

public:
    UIGeometricData()
        : scale(1.0f, 1.0f)
        , angle(0.0f)
        , cosA(1.0f)
        , sinA(0.0f)
        , calculatedAngle(0.0f)
    {
    }
    Vector2 position;
    Vector2 size;

    Vector2 pivotPoint;
    Vector2 scale;
    float32 angle;

    float32 cosA;
    float32 sinA;

    void AddGeometricData(const UIGeometricData &data)
    {
        position.x = data.position.x - data.pivotPoint.x * data.scale.x + position.x * data.scale.x;
        position.y = data.position.y - data.pivotPoint.y * data.scale.y + position.y * data.scale.y;
        if(data.angle != 0)
        {
            float tmpX = position.x;
            position.x = (tmpX - data.position.x) * data.cosA  + (data.position.y - position.y) * data.sinA + data.position.x;
            position.y = (tmpX - data.position.x) * data.sinA  + (position.y - data.position.y) * data.cosA + data.position.y;
        }
        scale.x *= data.scale.x;
        scale.y *= data.scale.y;
        angle += data.angle;
        if(angle != calculatedAngle)
        {
            if(angle != data.angle)
            {
                cosA = cosf(angle);
                sinA = sinf(angle);
            }
            else
            {
                cosA = data.cosA;
                sinA = data.sinA;
            }
            calculatedAngle = angle;
        }

        unrotatedRect.x = position.x - pivotPoint.x * scale.x;
        unrotatedRect.y = position.y - pivotPoint.y * scale.y;
        unrotatedRect.dx = size.x * scale.x;
        unrotatedRect.dy = size.y * scale.y;
    }

    DAVA_DEPRECATED(void AddToGeometricData(const UIGeometricData &data))
    {
        AddGeometricData(data);
    }

    void BuildTransformMatrix( Matrix3 &transformMatr ) const
    {
        Matrix3 pivotMatr;
        pivotMatr.BuildTranslation( -pivotPoint );

        Matrix3 translateMatr;
        translateMatr.BuildTranslation( position );

        Matrix3 rotateMatr;
        rotateMatr.BuildRotation( cosA, sinA );

        Matrix3 scaleMatr;
        scaleMatr.BuildScale( scale );

        transformMatr = pivotMatr * scaleMatr * rotateMatr * translateMatr;
    }

    void GetPolygon( Polygon2 &polygon ) const
    {
        polygon.Clear();
        polygon.points.reserve( 4 );
        polygon.AddPoint( Vector2() );
        polygon.AddPoint( Vector2( size.x, 0 ) );
        polygon.AddPoint( size );
        polygon.AddPoint( Vector2( 0, size.y ) );

        Matrix3 transformMtx;
        BuildTransformMatrix( transformMtx );
        polygon.Transform( transformMtx );
    }

    const Rect &GetUnrotatedRect() const
    {
        return unrotatedRect;
    }
    
    Rect GetAABBox() const
    {
        Polygon2 polygon;
        GetPolygon(polygon);

        AABBox2 aabbox;
        for(int32 i = 0; i < polygon.GetPointCount(); ++i)
        {
            aabbox.AddPoint(polygon.GetPoints()[i]);
        }
        Rect bboxRect = Rect(aabbox.min, aabbox.max - aabbox.min);
        return bboxRect;
    }

private:
    float32 calculatedAngle;
    Rect unrotatedRect;
};


    /**
     \ingroup controlsystem
     \brief Base control system unit.
        Responsible for update, draw and user input processing.

        Methods call sequence:
        When the control adds to the hierarchy:

            -if hierarchy is allready on the screen SystemWillAppear() will be called. SystemWillAppear()
                calls WillAppear() for the control and then calls SystemWillAppear() for all control children.

            -when the control adding to the hierarchy is done SystemDidAppear() and DidAppear() calls at the same way.

            -if hierarchy is not on the screen all methods would be called only when the hierarcy parent
                be placed to the screen.

        When the control removes from hierarchy:

            -SystemWillDisappear() will be called. SystemWillDisappear()
                calls WillDisappear() for the control and then calls SystemWillDisappear() for all control children.

            -when the control is removed from the hierarchy SystemDidDisappear() and DidDisappear() calls at the same way.

        Every frame:

            -SystemUpdate() is calls. SystemUpdate() calls Updadte() for the control then calls SystemUpdate()
                for the all control children.

            -SystemDraw() is calls. SystemDraw() calculates current control geometric data. Transmit information
                about the parent color to the control background. Sets clip if requested. Calls Draw().
                Calls SystemDraw() for the all control children. Calls DrawAfterChilds(). Returns clip back.
                Draw() method proceed control background drawing by default.
                You can't remove, add or sort controls on the draw step.

        Every input:

            -SystemInput() is calls. At the first control process SystemInput() for all their children. If one
                of the children returns true from their SystemInput(), control is returns true too. If no one
                of the children returns true control is returns result of the SystemProcessInput() method.

            -SystemProcessInput() method checks if the control is responsible to process current input. If input
                is possible to process, SystemProcessInput() sets system flags and calls Input() method, then returns true.
                If input is inpossible to process SystemProcessInput() returns false.

        Each control contain UIControlBackground object responsible for visual
        representation. UIControlBackground can be changed for the custom. Or you can
        just overload Draw() method for the custom drawing.
     */
class UIControl : public AnimatedObject
{
    friend class UIControlSystem;
    friend class UIScreenTransition;
public:
    /**
     \enum Control state bits.
     */
    enum eControlState
    {
        STATE_NORMAL            = 1 << 0,//!<Control isn't under influence of any activities.
        STATE_PRESSED_OUTSIDE   = 1 << 1,//!<Mouse or touch comes into control but dragged outside of control.
        STATE_PRESSED_INSIDE    = 1 << 2,//!<Mouse or touch comes into control.
        STATE_DISABLED          = 1 << 3,//!<Control is disabled (don't process any input). Use this state only if you want change graphical representation of the control. Don't use this state for the disabling inputs for parts of the controls hierarchy!.
        STATE_SELECTED          = 1 << 4,//!<Just a state for base control, nothing more.
        STATE_HOVER             = 1 << 5,//!<This bit is rise then mouse is over the control.

        STATE_COUNT             = 6
    };
    /**
     \enum Control events supported by default.
     */
    enum eEventType
    {
        EVENT_TOUCH_DOWN            = 1,//!<Trigger when mouse button or touch comes down inside the control.
        EVENT_TOUCH_UP_INSIDE       = 2,//!<Trigger when mouse pressure or touch processed by the control is released.
        EVENT_VALUE_CHANGED         = 3,//!<Used with sliders, spinners and switches. Trigger when value of the control is changed. Non-NULL callerData means that value is changed from code, not from UI.
        EVENT_HOVERED_SET           = 4,//!<
        EVENT_HOVERED_REMOVED       = 5,//!<
        EVENT_FOCUS_SET             = 6,//!<Trigger when control becomes focused
        EVENT_FOCUS_LOST            = 7,//!<Trigger when control losts focus
        EVENT_TOUCH_UP_OUTSIDE      = 8,//!<Trigger when mouse pressure or touch processed by the control is released outside of the control.
        EVENTS_COUNT
    };

    enum eDebugDrawPivotMode
    {
        DRAW_NEVER          = 1, //!<Never draw the Pivot Point.
        DRAW_ONLY_IF_NONZERO,    //!<Draw the Pivot Point only if it is defined (nonzero).
        DRAW_ALWAYS              //!<Always draw the Pivot Point mark.
    };

    friend class ControlSystem;


public:

    /**
     \brief Creates control with requested size and position.
     \param[in] rect Size and coordinates of control you want.
     \param[in] rectInAbsoluteCoordinates Send 'true' if you want to make control in screen coordinates.
        Rect coordinates will be recalculated to the hierarchy coordinates.
        Warning, rectInAbsoluteCoordinates isn't properly works for now!
     */
    UIControl(const Rect &rect = Rect(), bool rectInAbsoluteCoordinates = false);

    /**
     \brief Returns Sprite used for draw in the current UIControlBackground object.
        You can call this function directly for the controlBackgound.
     \returns Sprite used for draw.
     */
    virtual Sprite* GetSprite() const;
    /**
     \brief Returns Sprite frame used for draw in the current UIControlBackground object.
        You can call this function directly for the controlBackgound.
     \returns Sprite frame used for draw.
     */
    int32 GetFrame() const;
    /**
     \brief Returns draw type used for draw in the current UIControlBackground object.
        You can call this function directly for the controlBackgound.
     \returns Draw type used for draw.
     */
    virtual UIControlBackground::eDrawType GetSpriteDrawType() const;
    /**
     \brief Returns Sprite align used for draw in the current UIControlBackground object.
        You can call this function directly for the controlBackgound.
     \returns Sprite eAlign bit mask used for draw.
     */
    virtual int32 GetSpriteAlign() const;
    /**
     \brief Sets Sprite for the control UIControlBackground object.
     \param[in] spriteName Sprite path-name.
     \param[in] spriteFrame Sprite frame you want to use for draw.
     */
    virtual void SetSprite(const FilePath &spriteName, int32 spriteFrame);
    /**
     \brief Sets Sprite for the control UIControlBackground object.
     \param[in] newSprite Pointer for a Sprite.
     \param[in] spriteFrame Sprite frame you want to use for draw.
     */
    virtual void SetSprite(Sprite *newSprite, int32 spriteFrame);
    /**
     \brief Sets Sprite frame you want to use for draw for the control UIControlBackground object.
     \param[in] spriteFrame Sprite frame.
     */
    virtual void SetSpriteFrame(int32 spriteFrame);
    /**
     \brief Sets draw type you want to use the control UIControlBackground object.
     \param[in] drawType Draw type to use for drawing.
     */
    virtual void SetSpriteDrawType(UIControlBackground::eDrawType drawType);
    /**
     \brief Sets Sprite align you want to use for draw for the control UIControlBackground object.
     \param[in] drawAlign Sprite eAlign bit mask.
     */
    virtual void SetSpriteAlign(int32 align);

    /**
     \brief Sets background what will be used for draw.
        Background is cloned inside control.
     \param[in] newBg control background you want to use for draw.
     */
    virtual void SetBackground(UIControlBackground *newBg);
    /**
     \brief Returns current background used for draw.
     \returns background used for draw.
     */
    virtual UIControlBackground * GetBackground() const;

    /**
     \brief Sets left align of control relative to its parent.
     \param[in] align left align of control.
     */
    virtual void SetLeftAlign(int32 align);
    /**
     \brief Returns left align of control relative to its parent.
     \returns left align of control.
     */
    virtual int32 GetLeftAlign() const;
    /**
     \brief Sets horizontal central align of control relative to its parent.
     \param[in] align horizontal central align of control.
     */
    virtual void SetHCenterAlign(int32 align);
    /**
     \brief Returns horizontal central align of control relative to its parent.
     \returns horizontal central align of control.
     */
    virtual int32 GetHCenterAlign() const;
    /**
     \brief Sets right align of control relative to its parent.
     \param[in] align right align of control.
     */
    virtual void SetRightAlign(int32 align);
    /**
     \brief Returns right align of control relative to its parent.
     \returns right align of control.
     */
    virtual int32 GetRightAlign() const;
        /**
     \brief Sets top align of control relative to its parent.
     \param[in] align top align of control.
     */
    virtual void SetTopAlign(int32 align);
    /**
     \brief Returns top align of control relative to its parent.
     \returns top align of control.
     */
    virtual int32 GetTopAlign() const;
        /**
     \brief Sets vertical central align of control relative to its parent.
     \param[in] align l vertical central align of control.
     */
    virtual void SetVCenterAlign(int32 align);
    /**
     \brief Returns vertical central align of control relative to its parent.
     \returns vertical central align of control.
     */
    virtual int32 GetVCenterAlign() const;
        /**
     \brief Sets bottom align of control relative to its parent.
     \param[in] align bottom align of control.
     */
    virtual void SetBottomAlign(int32 align);
    /**
     \brief Returns bottom align of control relative to its parent.
     \returns bottom align of control.
     */
    virtual int32 GetBottomAlign() const;
    /**
     \brief Sets control ability to change left align.
     \param[in] isEnabled left align availability.
     */
    virtual void SetLeftAlignEnabled(bool isEnabled);
    /**
     \brief Returns availability of left align of control.
     \returns ability to change left align.
     */
    virtual bool GetLeftAlignEnabled() const;
    /**
     \brief Sets horizontal central align of control relative to its parent.
     \param[in] align horizontal central align of control.
     */
    virtual void SetHCenterAlignEnabled(bool isEnabled);
    /**
     \brief Returns horizontal central align of control relative to its parent.
     \returns horizontal central align of control.
     */
    virtual bool GetHCenterAlignEnabled() const;
    /**
     \brief Sets right align of control relative to its parent.
     \param[in] align right align of control.
     */
    virtual void SetRightAlignEnabled(bool isEnabled);
    /**
     \brief Returns right align of control relative to its parent.
     \returns right align of control.
     */
    virtual bool GetRightAlignEnabled() const;
        /**
     \brief Sets top align of control relative to its parent.
     \param[in] align top align of control.
     */
    virtual void SetTopAlignEnabled(bool isEnabled);
    /**
     \brief Returns top align of control relative to its parent.
     \returns top align of control.
     */
    virtual bool GetTopAlignEnabled() const;
        /**
     \brief Sets vertical central align of control relative to its parent.
     \param[in] align l vertical central align of control.
     */
    virtual void SetVCenterAlignEnabled(bool isEnabled);
    /**
     \brief Returns vertical central align of control relative to its parent.
     \returns vertical central align of control.
     */
    virtual bool GetVCenterAlignEnabled() const;
        /**
     \brief Sets bottom align of control relative to its parent.
     \param[in] align bottom align of control.
     */
    virtual void SetBottomAlignEnabled(bool isEnabled);
    /**
     \brief Returns bottom align of control relative to its parent.
     \returns bottom align of control.
     */
    virtual bool GetBottomAlignEnabled() const;

    /**
     \brief Returns untransformed control rect.
        To get control metrics that applies all control transformation you need to use
        geometric data received with GetGeometricData().
     \returns control rect.
     */
    inline Rect GetRect() const;

    /**
     \brief Returns absolute untransformed control rect.
        To get control metrics that applies all control transformation you need to use
        geometric data received with GetGeometricData().
     \returns control rect.
     */
    Rect GetAbsoluteRect();

    DAVA_DEPRECATED(Rect GetRect(bool absoluteCoordinates));

    /**
     \brief Sets the untransformed control rect.
     \param[in] rect new control rect.
     */
    virtual void SetRect(const Rect &rect);

    /**
     \brief Sets the untransformed control absolute rect.
     \param[in] rect new control absolute rect.
     */
    void SetAbsoluteRect(const Rect &rect);

    DAVA_DEPRECATED(virtual void SetRect(const Rect &rect, bool rectInAbsoluteCoordinates));

    /**
     \brief Returns untransformed control position.
        To get control metrics that applies all control transformation you need to use
        geometric data received with GetGeometricData().
     \returns control position.
     */
    inline const Vector2 &GetPosition() const;

    /**
     \brief Returns untransformed control position.
        To get control metrics that applies all control transformation you need to use
        geometric data received with GetGeometricData().
     \returns control absolute position.
     */
    Vector2 GetAbsolutePosition();

    DAVA_DEPRECATED(Vector2 GetPosition(bool absoluteCoordinates));

    /**
     \brief Sets the untransformed control position.
     \param[in] position new control position.
     */
    virtual void SetPosition(const Vector2 &position);

    /**
     \brief Sets the untransformed control absolute position.
     \param[in] position new control absolute position.
     */
    void SetAbsolutePosition(const Vector2 &position);

    DAVA_DEPRECATED(virtual void SetPosition(const Vector2 &position, bool positionInAbsoluteCoordinates));

    /**
     \brief Returns untransformed control size.
        To get control metrics that applies all control transformation you need to use
        geometric data received with GetGeometricData().
     \returns control size.
     */
    inline const Vector2 &GetSize() const;

    /**
     \brief Sets the untransformed control size.
     \param[in] newSize new control size.
     */
    virtual void SetSize(const Vector2 &newSize);

    /**
     \brief Returns control pivot point.
     \returns control pivot point.
     */
    inline const Vector2 &GetPivotPoint() const;

    /**
     \brief Sets the control pivot point.
     \param[in] newPivot new control pivot point.
     */
    inline void SetPivotPoint(const Vector2 &newPivot);

    /**
     \brief Returns control pivot.
     \returns control pivot.
     */
    inline Vector2 GetPivot() const;

    /**
     \brief Sets the control pivot.
     \param[in] newPivot new control pivot.
     */
    inline void SetPivot(const Vector2 &newPivot);

    /**
     \brief Returns control scale.
     \returns control scale.
     */
    inline const Vector2 &GetScale() const;

    /**
     \brief Sets the control scale.
     \param[in] newScale new control scale.
     */
    inline void SetScale(const Vector2 &newScale);

    /**
     \brief Returns actual control transformation and metrics.
     \returns control geometric data.
     */
    virtual const UIGeometricData &GetGeometricData(bool absoluteCoordinates = true);

    /**
     \brief Returns actual control local transformation and metrics.
     \returns control geometric data.
     */
    UIGeometricData GetLocalGeometricData() const;

    /**
     \brief Sets the scaled control rect.
        This method didn't apply any changes to the control size, but recalculate control scale.
     Warning, rectInAbsoluteCoordinates isn't properly works for now!
     \param[in] rect new control rect.
     */
    virtual void SetScaledRect(const Rect &rect, bool rectInAbsoluteCoordinates = false);

    /**
     \brief Returns control rotation angle in radians.
     \returns control angle in radians.
     */
    inline float32 GetAngle() const;
    inline float32 GetAngleInDegrees() const;
    
    /**
     \brief Sets contol rotation angle in radians.
        Control rotates around the pivot point.
     \param[in] angleInRad new control angle in radians.
     */
    virtual void SetAngle(float32 angleInRad);

    void SetAngleInDegrees(float32 angle);

    /**
     \brief Returns control visibility.
        Invisible controls don't process any inputs. But allows input processing for their children.
        Also for invisible controls didn't calls Draw() and DrawAfterChilds() methods.
        But this methods calls for their children.
     \returns control visibility.
     */
    DAVA_DEPRECATED(virtual bool GetVisible() const);// use GetRecursiveVisible instead

    /**
     \brief Sets contol visibility.
        Invisible controls don't process any inputs. But allows input processing for their children.
        Also for invisible controls didn't calls Draw() and DrawAfterChilds() methods.
        But this methods calls for their children.
        It's always better to remove part of controls hierarchy from the parent then to make them invisible.
        Visibility is usually used for the single control.
     \param[in] isVisible new control visibility.
     \param[in] hierarchic use true if you want to all control children change visiblity.
     */
    DAVA_DEPRECATED(virtual void SetVisible(bool isVisible, bool hierarchic = true));// use SetRecursiveVisible instead;

    /**
     \brief Returns control visibility.
        Invisible controls don't process any inputs.
        Also for invisible controls didn't calls Draw() and DrawAfterChilds() methods.
     \returns control visibility.
     */
    inline bool GetRecursiveVisible() const;

    /**
     \brief Sets contol recursive visibility.
        Invisible controls don't process any inputs.
        Also for invisible controls didn't calls Draw() and DrawAfterChilds() methods.
     \param[in] isVisible new control visibility.
     */
    virtual void SetRecursiveVisible(bool isVisible);

    /**
     \brief Returns control input processing ability.
        Be ware! Base control processing inputs by default.
     \returns true if control pocessing inputs.
     */
    inline bool GetInputEnabled() const;

    /**
     \brief Sets contol input processing ability.
        If input is disabled control don't process any inputs. If input is disabled all inputs events would comes to the parent control.
        Please use input enabling/disabling for the single controls or forthe small parts of hierarchy.
        It's always better to add transparent control that covers all screen and would process all
        incoming inputs to prevent input processing for the all screen controls or for the large part of hierarchy.
     \param[in] isEnabled is control should process inputs?
     \param[in] hierarchic use true if you want to all control children change input ability.
     */
    virtual void SetInputEnabled(bool isEnabled, bool hierarchic = true);

    /**
     \brief Returns control focusing ability.
     Be ware! Base control can be focused by default.
     \returns true if control can be focused.
     */
    inline bool GetFocusEnabled() const;

    /**
     \brief Sets contol focusing ability.
     If focus possibility is disabled control can't be focused. Disable focusing for scroll
     controls (like UIScrollView, UIScrollList, etc.)
     \param[in] isEnabled is control can be focused?
     */
    virtual void SetFocusEnabled(bool isEnabled);

    /**
     \brief Returns control enabling state.
        Disabled control don't process any inputs. But allows input processing for their children.
        Use this state only if you want change graphical representation of the control.
        Don't use this state for the disabling inputs for parts of the controls hierarchy!
        All controls is enabled by default.
     \returns true if control is disabled.
     */
    bool GetDisabled() const;

    /**
     \brief Sets the contol enabling/disabling.
        Disabled control don't process any inputs. But allows input processing for their children.
        Use this state only if you want change graphical representation of the control.
        Don't use this state for the disabling inputs for parts of the controls hierarchy!
        All controls is enabled by default.
     \param[in] isDisabled is control disabled?
     \param[in] hierarchic use true if you want to all control children change enabling/disabling.
     */
    virtual void SetDisabled(bool isDisabled, bool hierarchic = true);

    /**
     \brief Returns control selection state.
     \returns is control selected.
     */
    bool GetSelected() const;

    /**
     \brief Sets contol selection state.
        Selection state don't influence on any control activities.
     \param[in] isSelected is control selected?
     \param[in] hierarchic use true if you want to all control children change selection state.
     */
    virtual void SetSelected(bool isSelected, bool hierarchic = true);

    /**
     \brief Returns control clip contents state.
        Clip contents is disabled by default.
     \returns true if control rect clips draw and input areas of his children.
     */
    inline bool GetClipContents() const;
    /**
     \brief Sets clip contents state.
        If clip contents is enabled all incoming inputs for the control children processed only
        inside the control rect of parent. All children draw clips too.
        Clip contents is disabled by default.
     \param[in] isNeedToClipContents true if control should clips all children draw and input by his rect.
     */
    virtual void SetClipContents(bool isNeedToClipContents);

    /**
     \brief Returns control hover state.
        Only controlsa what processed inputs may be hovered.
     \returns control hover state is true if mouse placed over the control rect and no mous buttons is pressed.
     */
    bool GetHover() const;

    /**
     \brief Is exclusive input enabled.
        If control have exlusive input enabled and this control starts to process
        inputs. All inputs would be directed only to this control. But this control can
        process multiple number of inputs at a time.
        Exclusive input is disabled by default.
     \returns true if control supports exclusive input.
     */
    inline bool GetExclusiveInput() const;
    /**
     \brief Enables or disables control exclusive input.
        If control have exlusive input enabled and this control starts to process
        inputs. All inputs would be directed only to this control. But this control can
        process multiple number of inputs at a time.
        Exclusive input is disabled by default.
     \param[in] isExclusiveInput should control process inputs exclusively?
     \param[in] hierarchic use true if you want to all control children change exclusive input state.
     */
    virtual void SetExclusiveInput(bool isExclusiveInput, bool hierarchic = true);
    /**
     \brief Checks if control is multiple input enabled.
        If multiple input is enabled control can process all incoming inputs (Two or
        more touches for example). Otherwise control process only first incoming input.
        Multiply input is disabled by default.
     \returns true if control supports multyple inputs.
     */
    inline bool GetMultiInput() const;
    /**
     \brief Sets contol multi input processing.
        If multiple input is enabled control can process all incoming inputs (Two or
        more touches for example). Otherwise control process only first incoming input.
        Multiply input is disabled by default.
     \param[in] isMultiInput should control supports multiple inputs?
     \param[in] hierarchic use true if you want to all control children change multi nput support state.
     */
    virtual void SetMultiInput(bool isMultiInput, bool hierarchic = true);

    /**
     \brief Sets the contol name.
        Later you can find control by this name.
     \param[in] _name new control name.
     */
    void SetName(const String & _name);

    /**
     \brief Returns current name of the control.
     \returns control name.
     */
    inline const String & GetName() const;

    /**
     \brief Sets the contol tag.
     \param[in] tag new control tag.
     */
    void SetTag(int32 tag);

    /**
     \brief Returns current control tag.
     \returns control tag.
     */
    inline int32 GetTag() const;

    /**
     \brief Returns control with given name.
     \param[in] name requested control name.
     \param[in] recursive use true if you want fro recursive search.
     \returns first control with given name.
     */
    UIControl * FindByName(const String & name, bool recursive = true) const;

    UIControl * FindByPath(const String & path) const;

    /**
     \brief Returns control state bit mask.
     \returns control state.
     */
    inline int32 GetState() const;
    /**
     \brief Sets control state bit mask.
        Try to not use this method manually.
     \param[in] state new control bit mask.
     */
    void SetState(int32 state);

    /**
     \brief Returns control parent.
     \returns if contorl hasn't parent returns NULL.
     */
    UIControl *GetParent() const;

    /**
     \brief Returns list of control children.
     \returns list of control children.
     */
    const List<UIControl*> &GetChildren() const;
    /**
     \brief Returns list of control children without internal controls.
     \returns list of control children without internal controls.
     */
    virtual List<UIControl* >& GetRealChildren();

    /**
     \brief Returns the list of internal controls, which are editable
     \ and belongs to the same control.
     */
    virtual List<UIControl* > GetSubcontrols();

    /**
     \brief Returns whether this control is subcontrol of its parent.
     */
    virtual bool IsSubcontrol();

    /**
     \brief Add control as a child.
        Children draws in the sequence of adding. If child has another parent
        this child removes from the parrent firstly.
     \param[in] control control to add.
     */
    virtual void AddControl(UIControl *control);
    /**
     \brief Removes control from the children list.
        If child isn't present in the method owners list nothin happens.
     \param[in] control control to remove.
     */
    virtual void RemoveControl(UIControl *control);
    /**
     \brief Remove this control from its parent, if any.
     */
    virtual void RemoveFromParent();
    /**
     \brief Removes all children from the control.
     */
    virtual void RemoveAllControls();
    /**
     \brief Brings given child front.
        This child will be drawn at the top of the control children.
        If child isn't present in the owners list nothin happens.
     \param[in] _control control to bring front.
     */
    virtual void BringChildFront(UIControl *_control);
    /**
     \brief Brings given child back.
        This child will be drawn at the bottom of the control children.
        If child isn't present in the owners list nothin happens.
     \param[in] _control control to bring back.
     */
    virtual void BringChildBack(UIControl *_control);
    /**
     \brief Inserts given child before the requested.
     \param[in] _control control to insert.
     \param[in] _belowThisChild control to insert before. If this control isn't present in the
        children list new child adds at the top of the list.
     */
    virtual void InsertChildBelow(UIControl * _control, UIControl * _belowThisChild);
    /**
     \brief Inserts given child after the requested.
     \param[in] _control control to insert.
     \param[in] _aboveThisChild control to insert after. If this control isn't present in the
     children list new child adds at the top of the list.
     */
    virtual void InsertChildAbove(UIControl * _control, UIControl * _aboveThisChild);
    /**
     \brief Sends given child before the requested.
        If one of the given children isn't present in the owners list nothin happens.
     \param[in] _control control to move.
     \param[in] _belowThisChild control to sends before.
     */
    virtual void SendChildBelow(UIControl * _control, UIControl * _belowThisChild);
    /**
     \brief Sends given child after the requested.
        If one of the given children isn't present in the owners list nothin happens.
     \param[in] _control control to move.
     \param[in] _aboveThisChild control to sends after.
     */
    virtual void SendChildAbove(UIControl * _control, UIControl * _aboveThisChild);

    /**
     \brief Adds callback message for the event trigger.
     \param[in] eventType event type you want to process.
     \param[in] msg message should be calld when the event triggered.
     */
    void AddEvent(int32 eventType, const Message &msg);
    /**
     \brief Removes callback message for the event trigger.
     \param[in] eventType event type you want to remove.
     \param[in] msg message to remove.
     \returns true if event is removed.
     */
    bool RemoveEvent(int32 eventType, const Message &msg);
    /**
     \brief Function to remove all events from event dispatcher.
     \returns true if we removed something, false if not
     */
    bool RemoveAllEvents();

    /**
     \brief Send given event to the all subscribed objects.
     \param[in] eventType event type you want to process.
     */
    void PerformEvent(int32 eventType);
    /**
     \brief Send given event with given user data to the all subscribed objects.
     \param[in] eventType event type you want to process.
     \param[in] callerData data you want to send to the all messages.
     */
    void PerformEventWithData(int32 eventType, void *callerData);

    /**
     \brief Creates the absoulutely identic copy of the control.
     \returns control copy.
     */
    virtual UIControl *Clone();
    /**
     \brief Copies all contorl parameters from the sended control.
     \param[in] srcControl Source control to copy parameters from.
     */
    virtual void CopyDataFrom(UIControl *srcControl);


     //Animation helpers

    /**
     \brief Starts wait animation for the control.
     \param[in] time animation time.
     \param[in] track animation track. 0 by default.
     \returns Animation object
     */
    Animation *     WaitAnimation(float32 time, int32 track = 0);
    /**
     \brief Starts move and size animation for the control.
     \param[in] rect New control position and size.
     \param[in] time animation time.
     \param[in] interpolationFunc time interpolation method.
     \param[in] track animation track. 0 by default.
     \returns Animation object
     */
    Animation *     MoveAnimation(const Rect & rect, float32 time, Interpolation::FuncType interpolationFunc = Interpolation::LINEAR, int32 track = 0);
    /**
     \brief Starts move and scale animation for the control. Changing scale instead of size.
     \param[in] rect New control position and size.
     \param[in] time animation time.
     \param[in] interpolationFunc time interpolation method.
     \param[in] track animation track. 0 by default.
     \returns Animation object
     */
    Animation *     ScaledRectAnimation(const Rect & rect, float32 time, Interpolation::FuncType interpolationFunc = Interpolation::LINEAR, int32 track = 0);
    /**
     \brief Starts scale animation for the control. Changing scale instead of size.
     \param[in] newSize New control size.
     \param[in] time animation time.
     \param[in] interpolationFunc time interpolation method.
     \param[in] track animation track. 0 by default.
     \returns Animation object
     */
    Animation *     ScaledSizeAnimation(const Vector2 & newSize, float32 time, Interpolation::FuncType interpolationFunc = Interpolation::LINEAR, int32 track = 0);
    /**
     \brief Starts control position animation.
     \param[in] _position New control position.
     \param[in] time animation time.
     \param[in] interpolationFunc time interpolation method.
     \param[in] track animation track. 0 by default.
     \returns Animation object
     */
    Animation *     PositionAnimation(const Vector2 & _position, float32 time, Interpolation::FuncType interpolationFunc = Interpolation::LINEAR, int32 track = 0);
    /**
     \brief Starts control size animation.
     \param[in] _size New control size.
     \param[in] time animation time.
     \param[in] interpolationFunc time interpolation method.
     \param[in] track animation track. 0 by default.
     \returns Animation object
     */
    Animation *     SizeAnimation(const Vector2 & _size, float32 time, Interpolation::FuncType interpolationFunc = Interpolation::LINEAR, int32 track = 0);
    /**
     \brief Starts control scale animation.
     \param[in] newScale New control scale.
     \param[in] time animation time.
     \param[in] interpolationFunc time interpolation method.
     \param[in] track animation track. 0 by default.
     \returns Animation object
     */
    Animation *     ScaleAnimation(const Vector2 & newScale, float32 time, Interpolation::FuncType interpolationFunc = Interpolation::LINEAR, int32 track = 0);
    /**
     \brief Starts control rotation angle animation.
     \param[in] newAngle New control rotation angle.
     \param[in] time animation time.
     \param[in] interpolationFunc time interpolation method.
     \param[in] track animation track. 0 by default.
     \returns Animation object
     */
    Animation *     AngleAnimation(float32 newAngle, float32 time, Interpolation::FuncType interpolationFunc = Interpolation::LINEAR, int32 track = 0);
    /**
     \brief Starts input enabling switching animation. This animation changing control
        input enabling state on the next frame after the animation start.
     \param[in] touchable New input enabled value.
     \param[in] hierarhic Is value need to be changed in all coltrol children.
     \param[in] track animation track. 0 by default.
     \returns Animation object
     */
    Animation *     TouchableAnimation(bool touchable, bool hierarhic = true, int32 track = 0);
    /**
     \brief Starts control disabling animation. This animation changing control
        disable state on the next frame after the animation start.
     \param[in] disabled New control disabling value.
     \param[in] hierarhic Is value need to be changed in all coltrol children.
     \param[in] track animation track. 0 by default.
     \returns Animation object
     */
    Animation *     DisabledAnimation(bool disabled, bool hierarhic = true, int32 track = 0);
    /**
     \brief Starts control visible animation. This animation changing control visibility
        on the next frame after the animation start.
     \param[in] visible New control visible value.
     \param[in] hierarhic Is value need to be changed in all coltrol children.
     \param[in] track animation track. 0 by default.
     \returns Animation object
     */
    Animation *     VisibleAnimation(bool visible, bool hierarhic = true, int32 track = 0);
    /**
     \brief Starts control recursive visible animation. This animation changing control visibility
        on the next frame after the animation start.
     \param[in] visible New control recursive visible value.
     \param[in] track animation track. 0 by default.
     \returns Animation object
     */
    Animation *     RecursiveVisibleAnimation(bool visible, int32 track = 0);
    /**
     \brief Starts control removation animation. This animation removes control from the parent
     on the next frame  after the animation start.
     \param[in] track animation track. 0 by default.
     \returns Animation object
     */
    Animation *     RemoveControlAnimation(int32 track = 0);
    /**
     \brief Starts control color animation.
     \param[in] New control color.
     \param[in] animation time.
     \param[in] time interpolation method.
     \param[in] track animation track. 0 by default.
     \returns Animation object
     */
    Animation * ColorAnimation(const Color & finalColor, float32 time, Interpolation::FuncType interpolationFunc = Interpolation::LINEAR, int32 track = 0);

protected:
    void TouchableAnimationCallback(BaseObject * caller, void * param, void *callerData);
    void DisabledAnimationCallback(BaseObject * caller, void * param, void *callerData);
    void VisibleAnimationCallback(BaseObject * caller, void * param, void *callerData);
    void RecursiveVisibleAnimationCallback(BaseObject * caller, void * param, void *callerData);
    void RemoveControlAnimationCallback(BaseObject * caller, void * param, void *callerData);

public:

    /**
     \brief enabling or disabling dbug draw for the control.
     \param[in] _debugDrawEnabled New debug draw value.
     \param[in] hierarchic Is value need to be changed in all coltrol children.
     */
    void    SetDebugDraw(bool _debugDrawEnabled, bool hierarchic = false);
    void    SetDebugDrawColor(const Color& color);
    const Color &GetDebugDrawColor() const;

    /**
     \brief Set the draw pivot point mode for the control.
     \param[in] mode draw pivot point mode
     \param[in] hierarchic Is value need to be changed in all coltrol children.
     */
    void SetDrawPivotPointMode(eDebugDrawPivotMode mode, bool hierarchic = false);

public:

    /**
     \brief Called before control will be added to view hierarchy.
        Can be overrided for control additioanl functionality implementation.
        By default this method is empty.
     */
    virtual void WillAppear();
    /**
     \brief Called before control will be removed from the view hierarchy.
        Can be overrided for control additioanl functionality implementation.
        By default this method is empty.
     */
    virtual void WillDisappear();
    /**
     \brief Called when control added to view hierarchy.
        Can be overrided for control additioanl functionality implementation.
        By default this method is empty.
     */
    virtual void DidAppear();
    /**
     \brief Called when control removed from the view hierarchy.
        Can be overrided for control additioanl functionality implementation.
        By default this method is empty.
     */
    virtual void DidDisappear();
    /**
     \brief Called before control will be added to view hierarchy.
        Internal method used by ControlSystem. Can be overriden to prevent hierarchical call.
     */
    virtual void SystemWillAppear();
    /**
     \brief Called before control will be removed from the view hierarchy.
        Internal method used by ControlSystem. Can be overriden to prevent hierarchical call.
     */
    virtual void SystemWillDisappear();
    /**
     \brief Called when control added to view hierarchy.
        Internal method used by ControlSystem. Can be overriden to prevent hierarchical call.
     */
    virtual void SystemDidAppear();
    /**
     \brief Called when control removed from the view hierarchy.
        Internal method used by ControlSystem. Can be overriden to prevent hierarchical call.
     */
    virtual void SystemDidDisappear();

    /**
     \brief Called when screen size is changed.
        This method called for the currently active screen when the screen size is changed. Or called after WillAppear() for the other screens.
        Internal method used by ControlSystem. Can be overriden to prevent hierarchical call.
     \param[in] newFullScreenSize New full screen size in virtual coordinates.
        Rect may be larger when the virtual screen size. Rect x and y position may be smaller when 0.
     */
    virtual void SystemScreenSizeDidChanged(const Rect &newFullScreenRect);
    /**
     \brief Called when screen size is changed.
        This method called for the currently active screen when the screen size is changed. Or called after WillAppear() for the other screens.
     \param[in] newFullScreenSize New full screen size in virtual coordinates.
        Rect may be larger when the virtual screen size. Rect x and y position may be smaller when 0.
     */
    virtual void ScreenSizeDidChanged(const Rect &newFullScreenRect);

    /**
     \brief SystemUpdate() calls Updadte() for the control then SystemUpdate() calls for the all control children.
        Internal method used by ControlSystem. Can be overriden to prevent hierarchical call or adjust functionality.
     \param[in] timeElapsed Current frame time delta.
     */
    virtual void SystemUpdate(float32 timeElapsed);
    /**
     \brief Calls on every frame to process controls drawing.
        Firstly this method calls Draw() for the curent control. When SystemDraw() called for the every control child.
        And at the end DrawAfterChilds() called for current control.
        Internal method used by ControlSystem.
        Can be overriden to adjust draw hierarchy.
     \param[in] geometricData Parent geometric data.
     */
    virtual void SystemDraw(const UIGeometricData &geometricData);// Internal method used by ControlSystem

    /**
     \brief set parent draw color into control
     \param[in] parentColor draw color of parent background.
     */
    virtual void SetParentColor(const Color &parentColor);

    /**
     \brief Calls on every input event. Calls SystemInput() for all control children.
        If no one of the children is processed input. Calls ProcessInput() for the current control.
        Internal method used by ControlSystem.
     \param[in] currentInput Input information.
     \returns true if control processed this input.
     */
    virtual bool SystemInput(UIEvent *currentInput);
    /**
     \brief Process incoming input and if it's necessary calls Input() method for the control.
        Internal method used by ControlSystem.
     \param[in] currentInput Input information.
     \returns true if control processed this input.
     */
    virtual bool SystemProcessInput(UIEvent *currentInput);// Internal method used by ControlSystem
    /**
     \brief Calls when input processd by control is cancelled.
        Internal method used by ControlSystem.
     \param[in] currentInput Input information.
     */
    virtual void SystemInputCancelled(UIEvent *currentInput);

    /**
     \brief Called when control is set as the hovered (by the mouse) control.
     Internal method used by ControlSystem. Can be overriden only by the people ho knows UI architecture.
     */
    virtual void SystemDidSetHovered();
    /**
     \brief Called when control is not a hovered (by the mouse) control.
     Internal method used by ControlSystem. Can be overriden only by the people ho knows UI architecture.
     */
    virtual void SystemDidRemoveHovered();

    /**
     \brief Called when control is set as the hovered (by the mouse) control.
     Can be overriden to implement start hoverig reaction.
     */
    virtual void DidSetHovered();
    /**
     \brief Called when control is not a hovered (by the mouse) control.
     Can be overriden to implement end hoverig reaction.
     */
    virtual void DidRemoveHovered();


    /**
     \brief Calls on every input event coming to control.
        Should be overriden to implement custom input reaction.
        During one input processing step into control may come more then one input event.
        For example: Pressing began event and pressing ended or five conituous mose move events etc.
        Called only if control inputEnable is true.
     \param[in] currentInput Input information.
     */
    virtual void Input(UIEvent *currentInput);
    /**
     \brief Calls when input processd by control is cancelled.
        Should be overriden to implement custom input cancelling reaction.
     \param[in] currentInput Input information.
     */
    virtual void InputCancelled(UIEvent *currentInput);
    /**
     \brief Calls on every frame with frame delata time parameter.
        Should be overriden to implement perframe functionality.
        Default realization is empty.
     \param[in] timeElapsed Current frame time delta.
     */
    virtual void Update(float32 timeElapsed);
    /**
     \brief Calls on every frame to draw control.
        Can be overriden to implement custom draw functionality.
        Default realization is drawing UIControlBackground with requested parameters.
     \param[in] geometricData Control geometric data.
     */
    virtual void Draw(const UIGeometricData &geometricData);
    /**
     \brief Calls on every frame with UIGeometricData after all children is drawed.
        Can be overriden to implement after children drawing.
        Default realization is empty.
     \param[in] geometricData Control geometric data.
     */
    virtual void DrawAfterChilds(const UIGeometricData &geometricData);

protected:
    virtual void SystemWillBecomeVisible();
    virtual void SystemWillBecomeInvisible();

    virtual void WillBecomeVisible();
    virtual void WillBecomeInvisible();

public:

    /**
     \brief Save the control to YAML node and return it.
     */
    virtual bool LoadPropertiesFromYamlNode(const YamlNode *node, UIYamlLoader *loader);
    virtual bool SavePropertiesToYamlNode(YamlNode *node, UIControl *defaultControl, const UIYamlLoader *loader);

    /**
     \brief Called when this control and his children are loaded.
     */
    virtual void LoadFromYamlNodeCompleted() {};


    /**
     \brief Returns control in hierarchy status.
     \returns True if control in view hierarchy for now.
     */
    bool InViewHierarchy() const;

    /**
     \brief Returns control on screen status.
     \returns True if control visible now.
     */
    bool IsOnScreen() const;
    /**
     \brief Returns point status realtive to control .
     \param[in] point Point to check.
     \param[in] expandWithFocus Is area should be expanded with focus.
     \returns True if inside the control rect.
     */
    virtual bool IsPointInside(const Vector2 &point, bool expandWithFocus = false);

    virtual bool IsLostFocusAllowed(UIControl *newFocus);

    virtual void SystemOnFocusLost(UIControl *newFocus);

    virtual void SystemOnFocused();

    virtual void OnFocusLost(UIControl *newFocus);

    virtual void OnFocused();

    /// sets rect to match background sprite, also moves pivot point to center
    void SetSizeFromBg(bool pivotToCenter = true);

    virtual void UpdateLayout();
    // Recalculate the size and positions for the child controls according to their Align Options.
    void ApplyAlignSettingsForChildren();

    const String &GetControlClassName() const;
    // Access to Custom Control Type.
    const String &GetCustomControlClassName() const;
    void SetCustomControlClassName(const String& value);
    void ResetCustomControlClassName();

    // Find the control by name and add it to the list, if found.
    bool AddControlToList(List<UIControl*>& controlsList, const String& controlName, bool isRecursive = false);

    // Get/set the Initial State.
    int32 GetInitialState() const;
    void SetInitialState(int32 newState);

    // Get/set visible flag for UI editor. Should not be serialized.
    bool GetVisibleForUIEditor() const { return visibleForUIEditor; };
    virtual void SetVisibleForUIEditor(bool value, bool hierarchic = true);

    void DumpInputs(int32 depthLevel);
    
    BaseObject *GetCustomData() const;
    void SetCustomData(BaseObject *data);

public:
    //TODO: store geometric data in UIGeometricData
    Vector2 relativePosition;//!<position in the parent control.
    Vector2 size;//!<control size.

    Vector2 pivotPoint;//!<control pivot point. Top left control corner by default.
    Vector2 scale;//!<control scale. Scale relative to pivot point.
    float32 angle;//!<control rotation angle. Rotation around pivot point.

protected:
    UIControl *parent;
    List<UIControl*> childs;
    List<UIControl*> realChilds;

    UIControlBackground *background;
    int32 controlState;

    // boolean flags are grouped here to pack them together (see please DF-2149).
    bool exclusiveInput : 1;
    bool recursiveVisible : 1;
    bool visible : 1;
    bool clipContents : 1;
    bool debugDrawEnabled : 1;
    bool multiInput : 1;

    bool visibleForUIEditor : 1;

    // Enable align options
    bool leftAlignEnabled : 1;
    bool hcenterAlignEnabled : 1;
    bool rightAlignEnabled : 1;
    bool topAlignEnabled : 1;
    bool vcenterAlignEnabled : 1;
    bool bottomAlignEnabled : 1;

    bool isUpdated : 1;
    bool isIteratorCorrupted : 1;

    int32 inputProcessorsCount;

    int32 currentInputID;
    int32 touchesInside;
    int32 totalTouches;

    // Align options
    int32 leftAlign;
    int32 hcenterAlign;
    int32 rightAlign;
    int32 topAlign;
    int32 vcenterAlign;
    int32 bottomAlign;

    UIGeometricData tempGeometricData;

    EventDispatcher *eventDispatcher;

    Color debugDrawColor;

    eDebugDrawPivotMode drawPivotPointMode;

    // If this UI control represents Custom Control - its type is stored here.
    String customControlType;

    // Initial control's state which is stored on Yaml.
    int32 initialState;

    void SetParent(UIControl *newParent);

    virtual ~UIControl();

    void RegisterInputProcessor();
    void RegisterInputProcessors(int32 processorsCount);
    void UnregisterInputProcessor();
    void UnregisterInputProcessors(int32 processorsCount);

    void DrawDebugRect(const UIGeometricData &geometricData, bool useAlpha = false);
    void DrawPivotPoint(const Rect &drawRect);
    
private:
    BaseObject *customData;

private:
    String name;
    int32  tag;
    bool inputEnabled : 1;
    bool focusEnabled : 1;


    void RecalculateAlignProperties();
    void RecalculateChildsSize();
    void RecalculatePivotPoint(const Rect &newRect);

    float32 GetSizeX(UIControl *parent, int32 leftAlign, int32 rightAlign, bool useHalfParentSize = false);
    float32 GetSizeY(UIControl *parent, int32 topAlign, int32 bottomAlign, bool useHalfParentSize = false);

    float32 GetCenterX(UIControl *parent, int32 centerAlign, UIControl* child);
    float32 GetCenterY(UIControl *parent, int32 centerAlign, UIControl* child);

    float32 GetRelativeX(UIControl *parent, int32 align);
    float32 GetRelativeX(UIControl *parent, int32 align, UIControl* child, bool useHalfParentSize = false);
    float32 GetRelativeY(UIControl *parent, int32 align);
    float32 GetRelativeY(UIControl *parent, int32 align, UIControl* child, bool useHalfParentSize = false);
    
public:
    
    virtual int32 GetBackgroundComponentsCount() const;
    virtual UIControlBackground *GetBackgroundComponent(int32 index) const;
    virtual UIControlBackground *CreateBackgroundComponent(int32 index) const;
    virtual void SetBackgroundComponent(int32 index, UIControlBackground *bg);
    virtual String GetBackgroundComponentName(int32 index) const;
    
    virtual int32 GetInternalControlsCount() const;
    virtual UIControl *GetInternalControl(int32 index) const;
    virtual UIControl *CreateInternalControl(int32 index) const;
    virtual void SetInternalControl(int32 index, UIControl *control);
    virtual String GetInternalControlName(int32 index) const;
    virtual String GetInternalControlDescriptions() const;

    // for introspection
    bool GetEnabled() const {
        return !GetDisabled();
    }
    
    void SetEnabledNotHierarchic(bool enabled) {
        SetDisabled(!enabled, false);
    }
    
    bool GetNoInput() const {
        return !GetInputEnabled();
    }
    
    void SetNoInput(bool noInput) {
        SetInputEnabled(!noInput, false);
    }
    
    bool IsDebugDraw() const
    {
        return debugDrawEnabled;
    }
    
    void SetDebugDrawNotHierarchic(bool val)
    {
        SetDebugDraw(val, false);
    }
    
    INTROSPECTION_EXTEND(UIControl, AnimatedObject,
                         PROPERTY("name", "Name", GetName, SetName, I_SAVE | I_VIEW | I_EDIT)
                         PROPERTY("position", "Position", GetPosition, SetPosition, I_SAVE | I_VIEW | I_EDIT)
                         PROPERTY("size", "Size", GetSize, SetSize, I_SAVE | I_VIEW | I_EDIT)
                         PROPERTY("pivot", "Pivot", GetPivotPoint, SetPivotPoint, I_SAVE | I_VIEW | I_EDIT)
                         PROPERTY("angle", "Angle", GetAngleInDegrees, SetAngleInDegrees, I_SAVE | I_VIEW | I_EDIT)
                         PROPERTY("visible", "Visible", GetRecursiveVisible, SetRecursiveVisible, I_SAVE | I_VIEW | I_EDIT)
                         PROPERTY("enabled", "Enabled", GetEnabled, SetEnabledNotHierarchic, I_SAVE | I_VIEW | I_EDIT)
                         PROPERTY("clip", "Clip", GetClipContents, SetClipContents, I_SAVE | I_VIEW | I_EDIT)
                         PROPERTY("noInput", "No Input", GetNoInput, SetNoInput, I_SAVE | I_VIEW | I_EDIT)
                         PROPERTY("tag", "Tag", GetTag, SetTag, I_SAVE | I_VIEW | I_EDIT)
                         PROPERTY("initialState", "Initial State", GetInitialState, SetInitialState, I_SAVE | I_VIEW | I_EDIT)
                         
                         PROPERTY("leftAlignEnabled", "Left Align Enabled", GetLeftAlignEnabled, SetLeftAlignEnabled, I_SAVE | I_VIEW | I_EDIT)
                         PROPERTY("leftAlign", "Left Align", GetLeftAlign, SetLeftAlign, I_SAVE | I_VIEW | I_EDIT)

                         PROPERTY("rightAlignEnabled", "Right Align Enabled", GetRightAlignEnabled, SetRightAlignEnabled, I_SAVE | I_VIEW | I_EDIT)
                         PROPERTY("rightAlign", "Right Align", GetRightAlign, SetRightAlign, I_SAVE | I_VIEW | I_EDIT)

                         PROPERTY("bottomAlignEnabled", "Bottom Align Enabled", GetBottomAlignEnabled, SetBottomAlignEnabled, I_SAVE | I_VIEW | I_EDIT)
                         PROPERTY("bottomAlign", "Bottom Align", GetBottomAlign, SetBottomAlign, I_SAVE | I_VIEW | I_EDIT)

                         PROPERTY("topAlignEnabled", "Top Align Enabled", GetTopAlignEnabled, SetTopAlignEnabled, I_SAVE | I_VIEW | I_EDIT)
                         PROPERTY("topAlign", "Top Align", GetTopAlign, SetTopAlign, I_SAVE | I_VIEW | I_EDIT)

                         PROPERTY("hcenterAlignEnabled", "Horizontal Center Align Enabled", GetHCenterAlignEnabled, SetHCenterAlignEnabled, I_SAVE | I_VIEW | I_EDIT)
                         PROPERTY("hcenterAlign", "Horizontal Center Align", GetHCenterAlign, SetHCenterAlign, I_SAVE | I_VIEW | I_EDIT)

                         PROPERTY("vcenterAlignEnabled", "Vertical Center Align Enabled", GetVCenterAlignEnabled, SetVCenterAlignEnabled, I_SAVE | I_VIEW | I_EDIT)
                         PROPERTY("vcenterAlign", "Vertical Center Align", GetVCenterAlign, SetVCenterAlign, I_SAVE | I_VIEW | I_EDIT)
                         PROPERTY("debugDraw", "Debug Draw", IsDebugDraw, SetDebugDrawNotHierarchic, I_VIEW | I_EDIT)
                         );
};

const Vector2 & UIControl::GetPivotPoint() const
{
    return pivotPoint;
}

void UIControl::SetPivotPoint(const Vector2 &newPivot)
{
    pivotPoint = newPivot;
}

Vector2 UIControl::GetPivot() const
{
    Vector2 pivot;
    pivot.x = (size.x == 0.0f) ? 0.0f : (pivotPoint.x/size.x);
    pivot.y = (size.y == 0.0f) ? 0.0f : (pivotPoint.y/size.y);
    return pivot;
}

void UIControl::SetPivot(const Vector2 &newPivot)
{
    SetPivotPoint(size*newPivot);
}

const Vector2 & UIControl::GetScale() const
{
    return scale;
}

void UIControl::SetScale( const Vector2 &newScale )
{
    scale = newScale;
}

const Vector2 &UIControl::GetSize() const
{
    return size;
}

const Vector2 &UIControl::GetPosition() const
{
    return relativePosition;
}

float32 UIControl::GetAngle() const
{
    return angle;
}

float32 UIControl::GetAngleInDegrees() const
{
    return RadToDeg(angle);
}

const String & UIControl::GetName() const
{
    return name;
}

int32 UIControl::GetTag() const
{
    return tag;
}

Rect UIControl::GetRect() const
{
    return Rect(relativePosition - pivotPoint, size);
}

bool UIControl::GetRecursiveVisible() const
{
    return recursiveVisible;
}

bool UIControl::GetInputEnabled() const
{
    return inputEnabled;
}

bool UIControl::GetFocusEnabled() const
{
    return focusEnabled;
}

bool UIControl::GetClipContents() const
{
    return clipContents;
}

bool UIControl::GetExclusiveInput() const
{
    return exclusiveInput;
}

bool UIControl::GetMultiInput() const
{
    return multiInput;
}

int32 UIControl::GetState() const
{
    return controlState;
}
};

#endif
