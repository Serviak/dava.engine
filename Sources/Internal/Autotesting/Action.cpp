#include "Autotesting/Action.h"

#ifdef __DAVAENGINE_AUTOTESTING__

#include "Autotesting/AutotestingSystem.h"
#include "UI/UIControlSystem.h"
#include "UI/UIScreenManager.h"

#include <iterator>
#include <iostream>
#include <sstream>

namespace DAVA
{

Action::Action() : BaseObject()
    , isExecuted(false)
	, name("Action")
{
}
Action::~Action()
{
}

void Action::SetName(const String &actionName)
{
	name = actionName;
}

void Action::Update(float32 timeElapsed)
{
    if(!isExecuted)
    {
        isExecuted = TestCondition();

		//TODO: use ifdef for this debug feature?
		if(isExecuted)
		{
			DebugLog("EXECUTED", true);
		}
    }
}

void Action::Execute()
{
	DebugLog("EXECUTE", true);

    isExecuted = TestCondition();

	//TODO: use ifdef for this debug feature?
	if(isExecuted)
	{
		DebugLog("EXECUTED", true);
	}
}

void Action::DebugLog(const String &prefix, bool toAutotestingSystem)
{
	String logMsg = Dump();
	logMsg = Format("%s %s", prefix.c_str(), logMsg.c_str());
	if(toAutotestingSystem)
	{
		AutotestingSystem::Instance()->OnMessage(logMsg);
	}
	else
	{
		Logger::Debug("Action::DebugLog %s", logMsg.c_str());
	}
}

String Action::Dump()
{
	return name;
}

bool Action::TestCondition()
{
    return true;
}

//--------------------------------------------------------------------------

void Action::SetText(const Vector<String> &controlPath, const WideString &text)
{
    //TODO: use specific setters or any other way to get text from custom control
    // otherwise it is supported only for UIStaticText and UITextField classes
    UIControl* control = FindControl(controlPath);
    UIStaticText* staticText = dynamic_cast<UIStaticText*>(control);
    if(staticText)
    {
        staticText->SetText(text);
        //Logger::Debug("Action::SetText UIStaticText");
    }
    else
    {
        UITextField* textField = dynamic_cast<UITextField*>(control);
        if(textField)
        {
            textField->SetText(text);
            //Logger::Debug("Action::SetText UITextField");
        }
        else
        {
            Logger::Debug("Action::SetText FAILED");
        }
    }
}

WideString Action::GetText(const Vector<String> &controlPath)
{
    //TODO: use specific getters or any other way to get text from custom control
    // otherwise it is supported only for UIStaticText and UITextField classes
    WideString text = L"";
    UIControl* control = FindControl(controlPath);
    UIStaticText* staticText = dynamic_cast<UIStaticText*>(control);
    if(staticText)
    {
        text = staticText->GetText();
    }
    else
    {
        UITextField* textField = dynamic_cast<UITextField*>(control);
        if(textField)
        {
            text = textField->GetText();
        }
    }
    return text;
}

//----------------------------------------------------------------------
// helpers
void Action::ProcessInput(const UIEvent &input)
{
    Logger::Debug("Action::ProcessInput %d phase=%d count=%d point=(%f, %f) physPoint=(%f,%f) key=%c",input.tid, input.phase, input.tapCount, input.point.x, input.point.y, input.physPoint.x, input.physPoint.y, input.keyChar);

    Vector<UIEvent> emptyTouches;
    Vector<UIEvent> touches;
    touches.push_back(input);
    UIControlSystem::Instance()->OnInput(0, emptyTouches, touches);

    AutotestingSystem::Instance()->OnInput(input);
}

UIControl* Action::FindControl(const Vector<String>& controlPath)
{
	if(UIControlSystem::Instance()->GetLockInputCounter() > 0) return NULL;

    UIControl* control = NULL;
    if(UIScreenManager::Instance()->GetScreen() && (!controlPath.empty()))
    {
        control = FindControl(UIScreenManager::Instance()->GetScreen(), controlPath[0]);

        for(uint32 i = 1; i < controlPath.size(); ++i)
        {
            if(!control) break;
            control = FindControl(control, controlPath[i]);
        }
    }
    return control;
}

UIControl* Action::FindControl(UIControl* srcControl, const String &controlName)
{
	if(UIControlSystem::Instance()->GetLockInputCounter() > 0) return NULL;

    if(srcControl)
    {
        int32 index = atoi(controlName.c_str());
        if((index == 0) && (Format("%d",index) != controlName))
        {
            // not number
            return srcControl->FindByName(controlName);
        }
        else
        {
            // number
            UIList* list = dynamic_cast<UIList*>(srcControl);
            if(list)
            {
                return FindControl(list, index);
            }
            else
            {
                return FindControl(srcControl, index);
            }
        }
    }
    return NULL;
}

UIControl* Action::FindControl(UIControl* srcControl, int32 index)
{
	if(UIControlSystem::Instance()->GetLockInputCounter() > 0) return NULL;

    if(srcControl)
    {
        const List<UIControl*> children = srcControl->GetChildren();
        int32 childIndex = 0;
        for(List<UIControl*>::const_iterator it = children.begin(); it != children.end(); ++it, ++childIndex)
        {
            if(childIndex == index)
            {
                return (*it);
            }
        }
    }
    return NULL;
}

UIControl* Action::FindControl(UIList* srcList, int32 index)
{
	if(UIControlSystem::Instance()->GetLockInputCounter() > 0) return NULL;

    if(srcList)
    {
        const List<UIControl*> &cells = srcList->GetVisibleCells();
        for(List<UIControl*>::const_iterator it = cells.begin(); it != cells.end(); ++it)
        {
            UIListCell* cell = dynamic_cast<UIListCell*>(*it);
            if(cell)
            {
                if(cell->GetIndex() == index && IsInside(srcList, cell))
                {
                    return cell;
                }
            }
        }
    }
    return NULL;
}

Vector2 Action::FindControlPosition(const Vector<String>& controlPath)
{
    Vector2 point;
    UIControl* control = FindControl(controlPath);
    if(control)
    {
        Rect controlRect(control->GetGeometricData().GetUnrotatedRect());
        point = Vector2(controlRect.x + 0.5f*controlRect.dx, controlRect.y + 0.5f*controlRect.dy);
    }
    return point;
}

bool Action::IsInside(UIControl* parent, UIControl* child)
{
    bool isInside = false;
    if(parent && child)
    {
        Rect parentRect(parent->GetGeometricData().GetUnrotatedRect());
        Rect childRect(child->GetGeometricData().GetUnrotatedRect());

		if(childRect.dx <= parentRect.dx && childRect.dy <= parentRect.dy)
		{
			// can be totally inside parent
			float32 insidePartX = 1.0f;
			float32 insidePartY = 1.0f;
			
			// for list and cell case we should scroll more on screen
// 			if(2*childRect.dx <= parentRect.dx)
// 			{
// 				insidePartX *= 2;
// 			}
// 			if(2*childRect.dy <= parentRect.dy)
// 			{
// 				insidePartY *= 2;
// 			}

			float32 offsetPartX = (1.0f - insidePartX)*0.5f;
			float32 offsetPartY = (1.0f - insidePartY)*0.5f;

			Rect testInsideRect(childRect.x + offsetPartX*childRect.dx, childRect.y + offsetPartY*childRect.dy, insidePartX*childRect.dx, insidePartY*childRect.dy);

			isInside = ((parentRect.x <= testInsideRect.x) && (testInsideRect.x + testInsideRect.dx <= parentRect.x + parentRect.dx) &&
            (parentRect.y <= testInsideRect.y) && (testInsideRect.y + testInsideRect.dy <= parentRect.y + parentRect.dy));
		}
		else
		{
			// check if child center is inside parent rect
			Vector2 childCenter(childRect.x + childRect.dx/2, childRect.y + childRect.dy/2);
			isInside = ((parentRect.x <= childCenter.x) && (childCenter.x <= parentRect.x + parentRect.dx) &&
            (parentRect.y <= childCenter.y) && (childCenter.y <= parentRect.y + parentRect.dy));
		}


        //isInside = ((parentRect.x <= childRect.x) && (childRect.x + childRect.dx <= parentRect.x + parentRect.dx) &&
        //    (parentRect.y <= childRect.y) && (childRect.y + childRect.dy <= parentRect.y + parentRect.dy));
    }
    return isInside;
}

String Action::PathToString(const Vector<String>& controlPath)
{
    Vector<String> controlPathFormatted;
    int32 pathSize = controlPath.size();
    controlPathFormatted.push_back("[");
    for(int32 i = 0; i < pathSize; ++i)
    {
        controlPathFormatted.push_back(controlPath[i]);
        if(i < pathSize - 1)
        {
            controlPathFormatted.push_back(",");
        }
    }
    controlPathFormatted.push_back("]");

    std::stringstream ss; 
    //std::copy(controlPath.begin(), controlPath.end(), std::ostream_iterator<String>(ss)); 
    std::copy(controlPathFormatted.begin(), controlPathFormatted.end(), std::ostream_iterator<String>(ss));
    return ss.str();
}

//----------------------------------------------------------------------
KeyPressAction::KeyPressAction(char16 _keyChar) : Action()
    , keyChar(_keyChar)
{
    SetName("KeyPressAction");
}

KeyPressAction::~KeyPressAction()
{
}

void KeyPressAction::Execute()
{
    KeyPress(keyChar);
    Action::Execute();
}

void KeyPressAction::KeyPress(char16 keyChar)
{
    //TODO: KeyPress
    //Logger::Debug("KeyPressAction::KeyPress %c", keyChar);
    UIEvent keyPress;
    keyPress.tid = 0;
    keyPress.phase = UIEvent::PHASE_KEYCHAR;
    keyPress.tapCount = 1;

    ProcessInput(keyPress);
}

String KeyPressAction::Dump()
{
	String baseStr = Action::Dump();
	return Format("%s key=%c", baseStr.c_str(), keyChar);
}

//----------------------------------------------------------------------

WaitAction::WaitAction(float32 _waitTime) : Action()
    , waitTime(_waitTime)
{
    SetName("WaitAction");
}

WaitAction::~WaitAction()
{

}

void WaitAction::Execute()
{
    Action::Execute();
}

void WaitAction::Update(float32 timeElapsed)
{
    waitTime -= timeElapsed;
    Action::Update(timeElapsed);
}

bool WaitAction::TestCondition()
{
    return (waitTime <= 0.0f);
}

String WaitAction::Dump()
{
	String baseStr = Action::Dump();
	return Format("%s time=%.2f", baseStr.c_str(), waitTime);
}

//----------------------------------------------------------------------

WaitForScreenAction::WaitForScreenAction(const String &_screenName, float32 timeout) : WaitAction(timeout)
	, screenName(_screenName)
{
    SetName("WaitForScreenAction");
}

WaitForScreenAction::~WaitForScreenAction()
{
}

void WaitForScreenAction::Execute()
{
    WaitAction::Execute();
}

bool WaitForScreenAction::TestCondition()
{
    if(WaitAction::TestCondition())
    {
        AutotestingSystem::Instance()->OnError(Format("WaitForScreenAction %s timeout", screenName.c_str()));
        return true;
    }
    if(UIControlSystem::Instance()->GetScreen())
    {
        return (UIControlSystem::Instance()->GetScreen()->GetName() == screenName);
    }
	return false;
}

String WaitForScreenAction::Dump()
{
	String baseStr = WaitAction::Dump();
	return Format("%s screenName=%s", baseStr.c_str(), screenName.c_str());
}


//----------------------------------------------------------------------

WaitForUIAction::WaitForUIAction(const String &_controlName, float32 timeout) : WaitAction(timeout)
{
    SetName("WaitForUIAction");
    controlPath.push_back(_controlName);
}

WaitForUIAction::WaitForUIAction(const Vector<String> &_controlPath, float32 timeout) : WaitAction(timeout)
    , controlPath(_controlPath)
{
    SetName("WaitForUIAction");
}

WaitForUIAction::~WaitForUIAction()
{
}

void WaitForUIAction::Execute()
{
    WaitAction::Execute();
}

bool WaitForUIAction::TestCondition()
{
    if(WaitAction::TestCondition())
    {
        AutotestingSystem::Instance()->OnError(Format("WaitForUIAction %s timeout", PathToString(controlPath).c_str()));
        return true;
    }
    return (FindControl(controlPath) != NULL);
}

String WaitForUIAction::Dump()
{
	String baseStr = WaitAction::Dump();
	String controlPathStr = PathToString(controlPath);
	return Format("%s controlPath=%s", baseStr.c_str(), controlPathStr.c_str());
}

//----------------------------------------------------------------------

SetTextAction::SetTextAction(const String& _controlName, const WideString &_text)
    : Action()
    , text(_text)
{
    SetName("SetTextAction");
    controlPath.push_back(_controlName);
}

SetTextAction::SetTextAction(const Vector<String>& _controlPath, const WideString &_text)
    : Action()
    , text(_text)
    , controlPath(_controlPath)
{
    SetName("SetTextAction");
}

SetTextAction::~SetTextAction()
{
}

void SetTextAction::Execute()
{
    Action::SetText(controlPath, text);
    Action::Execute();
}

String SetTextAction::Dump()
{
	String baseStr = Action::Dump();
	String controlPathStr = PathToString(controlPath);
	return Format("%s controlPath=%s text=%s", baseStr.c_str(), controlPathStr.c_str(), text.c_str());
}

//----------------------------------------------------------------------


};

#endif //__DAVAENGINE_AUTOTESTING__