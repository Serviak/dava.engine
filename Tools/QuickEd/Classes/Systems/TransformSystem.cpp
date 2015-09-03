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

#include "Input/InputSystem.h"
#include "Input/KeyboardDevice.h"

#include "Systems/TransformSystem.h"
#include "Document.h"
#include "Defines.h"
#include "UI/UIEvent.h"
#include "Input/KeyboardDevice.h"
#include "Model/PackageHierarchy/ControlNode.h"
#include "Model/ControlProperties/RootProperty.h"
#include "Model/ControlProperties/ValueProperty.h"
#include "UI/QtModelPackageCommandExecutor.h"

using namespace DAVA;

namespace
{
    const Array<Array<int, Vector2::AXIS_COUNT>, ControlAreaInterface::CORNERS_COUNT> cornersDirection = 
    { {
        {{ -1, -1 }}, // TOP_LEFT_AREA
        {{ 0, -1 }}, // TOP_CENTER_AREA
        {{ 1, -1 }}, //TOP_RIGHT_AREA
        {{ -1, 0 }}, //CENTER_LEFT_AREA
        {{ 1, 0 }}, //CENTER_RIGHT_AREA
        {{ -1, 1 }}, //BOTTOM_LEFT_AREA
        {{ 0, 1 }}, //BOTTOM_CENTER_AREA
        {{ 1, 1 }}  //BOTTOM_RIGHT_AREA
    } };
}

TransformSystem::TransformSystem(Document *parent)
    : BaseSystemClass(parent)
    , steps({ { 10, 20, 20 } }) //10 for rotate and 20 for move/resize
{
    accumulates.fill({ { 0, 0 } });
}

void TransformSystem::MouseEnterArea(ControlNode *targetNode, const eArea area)
{
    activeControl = targetNode;
    activeArea = area;
}

void TransformSystem::MouseLeaveArea()
{
    activeControl = nullptr;
    activeArea = NO_AREA;
}

bool TransformSystem::OnInput(UIEvent* currentInput)
{
    switch(currentInput->phase)
    {
        case UIEvent::PHASE_KEYCHAR:
            return ProcessKey(currentInput->tid);
        case UIEvent::PHASE_BEGAN:
            prevPos = currentInput->point;
            return activeArea != NO_AREA;
        case UIEvent::PHASE_DRAG:
            return ProcessDrag(currentInput->point);
        case UIEvent::PHASE_ENDED:
        {
            accumulates.fill({ { 0, 0 } });
            //return true if we did transformation
            if (dragRequested)
            {
                dragRequested = false;
                return true;
            }
            return false;
        }
        default:
            return false;
            
    }
}

void TransformSystem::OnSelectionWasChanged(const SelectedControls &selected, const SelectedControls &deselected)
{
    UniteSets(selected, selectedControls);
    SubstractSets(deselected, selectedControls);
}

void TransformSystem::Detach()
{
    accumulates.fill({ { 0, 0 } });
}

bool TransformSystem::ProcessKey(const int32 key)
{
    if(!selectedControls.empty())
    {
        switch(key)
        {
            case DVKEY_LEFT:
                MoveAllSelectedControls(Vector2(-1.0f, 0.0f));
                return true;
            case DVKEY_UP:
                MoveAllSelectedControls(Vector2(0.0f, -1.0f));
                return true;
            case DVKEY_RIGHT:
                MoveAllSelectedControls(Vector2(1.0f, 0.0f));
                return true;
            case DVKEY_DOWN:
                MoveAllSelectedControls(Vector2(0.0f, 1.0f));
                return true;
            default:
                return false;
        }
    }
    return false;
}

bool TransformSystem::ProcessDrag(const Vector2 &pos)
{
    if(activeArea == NO_AREA)
    {
        return false;
    }
    dragRequested = true;
    const auto &keyBoard = InputSystem::Instance()->GetKeyboard();
    bool retval = true;
    auto gd =  activeControl->GetControl()->GetGeometricData();

    switch(activeArea)
    {
        case FRAME_AREA:
            MoveConrol(pos);
            break;
        case TOP_LEFT_AREA:
        case TOP_CENTER_AREA:
        case TOP_RIGHT_AREA:
        case CENTER_LEFT_AREA:
        case CENTER_RIGHT_AREA:
        case BOTTOM_LEFT_AREA:
        case BOTTOM_CENTER_AREA:
        case BOTTOM_RIGHT_AREA:
        {
            bool withPivot = keyBoard.IsKeyPressed(DVKEY_ALT);
            bool rateably = keyBoard.IsKeyPressed(DVKEY_SHIFT);
            ResizeControl(pos, withPivot, rateably);
        }
        break;
        case PIVOT_POINT_AREA:
        {
            auto control = activeControl->GetControl();
            Vector2 delta = pos - prevPos;
            DVASSERT(gd.scale.x != 0.0f && gd.scale.y != 0.0f);
            Vector2 scaledDelta = delta / gd.scale;
            //position calculates in absolute coordinates
            AdjustProperty(activeControl, "Position", scaledDelta);
            //pivot point calculate in rotate coordinates
            Vector2 angeledDelta(scaledDelta.x * gd.cosA + scaledDelta.y * gd.sinA,
                        scaledDelta.x * -gd.sinA + scaledDelta.y * gd.cosA);
            Vector2 pivot(angeledDelta / control->GetSize());
            AdjustProperty(activeControl, "Pivot", pivot);
        }
            break;
        case ROTATE_AREA:
        {
            auto control = activeControl->GetControl();
            const Rect &ur = gd.GetUnrotatedRect();
            Vector2 pivotPoint = ur.GetPosition() + ur.GetSize() * control->GetPivot();
            Vector2 rotatePoint = pivotPoint;
            Vector2 l1(prevPos - rotatePoint);
            Vector2 l2(pos - rotatePoint);
            float32 angleRad = atan2(l2.y, l2.x) - atan2(l1.y, l1.x);
            float32 angle = angleRad * 180.0f / PI;
            if (InputSystem::Instance()->GetKeyboard().IsKeyPressed(DVKEY_SHIFT))
            {
                Vector2 angle2(angle, angle);
                AccumulateOperation(ROTATE_OPERATION, angle2);
                angle = angle2.dx;
            }
            else
            {
                angle = round(angle);
            }
            AdjustProperty(activeControl, "Angle", angle);
        
        }
            break;
        default:
            retval = false;
    }
    prevPos = pos;
    return retval;
}

void TransformSystem::MoveAllSelectedControls(const Vector2 &delta)
{
    for( auto &controlNode : selectedControls)
    {
        if(controlNode->IsEditingSupported())
        {
            AdjustProperty(controlNode, "Position", delta);
        }
    }
}

void TransformSystem::MoveConrol(const Vector2& pos)
{
    Vector2 delta = pos - prevPos;
    auto gd =  activeControl->GetControl()->GetGeometricData();
    Vector2 realDelta = delta / gd.scale;
    if (InputSystem::Instance()->GetKeyboard().IsKeyPressed(DVKEY_SHIFT))
    {
        AccumulateOperation(MOVE_OPERATION, realDelta);
    }
    AdjustProperty(activeControl, "Position", realDelta);
}

void TransformSystem::ResizeControl(const Vector2& pos, bool withPivot, bool rateably)
{
    DVASSERT(activeArea != NO_AREA);
    Vector2 delta = pos - prevPos;
    if(delta.x == 0.0f && delta.y == 0.0f)
    {
        return;
    }
    const auto invertX = cornersDirection.at(activeArea)[Vector2::AXIS_X];
    const auto invertY = cornersDirection.at(activeArea)[Vector2::AXIS_Y];

    auto gd = activeControl->GetControl()->GetGeometricData();
    Vector2 angeledDelta(delta.x * cosf(gd.angle) + delta.y * sinf(gd.angle),
                         delta.x * -sinf(gd.angle) + delta.y * cosf(gd.angle)); //rotate delta
     //scale rotated delta
    DVASSERT(gd.scale.x != 0.0f && gd.scale.y != 0.0f);
    Vector2 realDelta(angeledDelta / gd.scale);
    Vector2 deltaPosition(realDelta);
    Vector2 deltaSize(realDelta);
    //make resize absolutely
    deltaSize.x *= invertX;
    deltaSize.y *= invertY;
    //disable move if not accepted
    if(invertX == 0)
    {
        deltaPosition.x = 0.0f;
    }
    if(invertY == 0)
    {
        deltaPosition.y = 0.0f;
    }

    auto pivotProp = document->GetPropertyByName(activeControl, "Pivot");
    DVASSERT(nullptr != pivotProp);
    Vector2 pivot = pivotProp->GetValue().AsVector2();

    //calculate new positionp
    deltaPosition.x *= invertX == -1 ? 1 - pivot.x : pivot.x;
    deltaPosition.y *= invertY == -1 ? 1 - pivot.y : pivot.y;
    
    //modify if pivot modificator selected
    if(withPivot)
    {
        deltaPosition.SetZero();
        auto pivotDeltaX = invertX == -1 ? pivot.x : 1.0f - pivot.x;
        if (pivotDeltaX != 0.0f)
        {
            deltaSize.x /= pivotDeltaX;
        }
        auto pivotDeltaY = invertY == -1 ? pivot.y : 1.0f - pivot.y;
        if (pivotDeltaY != 0.0f)
        {
            deltaSize.y /= pivotDeltaY;
        }

    }
    //modify rateably
    if(rateably)
    {
        //check situation when we try to resize up and down simultaneously
        if(invertX != 0 && invertY != 0) //actual only for corners
        {
            if(fabs(angeledDelta.x) > 0.0f && fabs(angeledDelta.y) > 0.0f) //only if up and down requested
            {
                bool canNotResize = ((angeledDelta.x * invertX) > 0.0f) ^ ((angeledDelta.y * invertY) > 0.0f);
                if(canNotResize) // and they have different sign for corner
                {
                    float prop = fabs(angeledDelta.x) / fabs(angeledDelta.y);
                    if(prop > 0.48f && prop < 0.52f) // like "resize 10 to up and 10 to down rateably"
                    {
                        return;
                    }
                }
            }
        }
        //calculate proportion of control
        const Vector2 &size = activeControl->GetControl()->GetSize();
        float proportion = size.y != 0.0f  ? size.x / size.y : 0.0f;
        if (proportion != 0.0f)
        {
            //get current drag direction
            if (fabs(angeledDelta.y) > fabs(angeledDelta.x))
            {
                deltaSize.x = deltaSize.y * proportion;
                if(!withPivot)
                {
                    deltaPosition.x = deltaSize.y * proportion;
                    if(invertX == 0)
                    {
                        deltaPosition.x *= (0.5f - pivot.x) * -1.0f; //rainbow unicorn was here and add -1 to the right.
                    }
                    else
                    {
                        deltaPosition.x *= (invertX == -1 ? 1.0f - pivot.x : pivot.x) * invertX;
                    }
                }
            }
            else
            {
                deltaSize.y = deltaSize.x / proportion;
                if(!withPivot)
                {
                    deltaPosition.y =  deltaSize.x / proportion;
                    if(invertY == 0)
                    {
                        deltaPosition.y *= (0.5f - pivot.y) * -1.0f; // another rainbow unicorn adds -1 here.
                    }
                    else
                    {
                        deltaPosition.y *= (invertY == -1 ? 1.0f - pivot.y : pivot.y) * invertY;
                    }
                }
            }
        }
    }
    
    //rotate delta position backwards, because SetPosition require absolute coordinates
    Vector2 rotatedPosition;
    rotatedPosition.x = deltaPosition.x * cosf(-gd.angle) + deltaPosition.y * sinf(-gd.angle);
    rotatedPosition.y = deltaPosition.x * -sinf(-gd.angle) + deltaPosition.y * cosf(-gd.angle);
    AdjustProperty(activeControl, "Position", rotatedPosition);

    AdjustProperty(activeControl, "Size", deltaSize);
}

template <typename T>
void TransformSystem::AdjustProperty(ControlNode *node, const String &propertyName, const T &delta)
{

    AbstractProperty *property = document->GetPropertyByName(node, propertyName);
    DVASSERT(nullptr != property);
    VariantType var(delta);
    
    switch(var.GetType())
    {
        case VariantType::TYPE_VECTOR2:
            var = VariantType(property->GetValue().AsVector2() + delta);
            break;
        case VariantType::TYPE_FLOAT:
            var = VariantType(property->GetValue().AsFloat() + delta);
            break;
        default:
            DVASSERT_MSG(false, "unexpected type");
            break;
    }
    
    document->GetCommandExecutor()->ChangeProperty(node, property, var);
}

void TransformSystem::AccumulateOperation(ACCUMULATE_OPERATIONS operation, DAVA::Vector2& delta)
{
    const int step = steps[operation];
    int &x = accumulates[operation][Vector2::AXIS_X];
    int &y = accumulates[operation][Vector2::AXIS_Y];
    if (abs(x) < step)
    {
        x += delta.x;
    }
    if (abs(y) < step)
    {
        y += delta.y;
    }
    delta = Vector2(x - x % step, y - y % step);
    x -= delta.x;
    y -= delta.y;
}