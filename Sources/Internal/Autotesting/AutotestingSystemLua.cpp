#include "Autotesting/AutotestingSystemLua.h"

#ifdef __DAVAENGINE_AUTOTESTING__

#include "AutotestingSystem.h"
#include "AutotestingDB.h"

#include "Utils/Utils.h"

extern "C"{
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
};

// directly include wrapped module here to compile only if __DAVAENGINE_AUTOTESTING__ is defined
//#include "AutotestingSystem_wrap.cxx"

//#include "VectorSwig.cpp"
//#include "UIControlSwig.cpp"
//#include "AutotestingSystemSwig.cpp"

extern "C" int luaopen_AutotestingSystem(lua_State *l);
extern "C" int luaopen_UIControl(lua_State *l); 
extern "C" int luaopen_Rect(lua_State *l);
extern "C" int luaopen_Vector(lua_State *l);
extern "C" int luaopen_KeyedArchive(lua_State *l);

namespace DAVA
{
        
AutotestingSystemLua::AutotestingSystemLua() : luaState(NULL), delegate(NULL), autotestingLocalizationSystem(NULL)
{
    autotestingLocalizationSystem = new LocalizationSystem();
}

AutotestingSystemLua::~AutotestingSystemLua()
{
    SafeRelease(autotestingLocalizationSystem);
    
    if(luaState)
    {
        lua_close(luaState);
        luaState = NULL;
    }
}

void AutotestingSystemLua::SetDelegate(AutotestingSystemLuaDelegate *_delegate)
{
    delegate = _delegate;
}
    
void AutotestingSystemLua::InitFromFile(const String &luaFilePath)
{
    Logger::Debug("AutotestingSystemLua::InitFromFile luaFilePath=%s", luaFilePath.c_str());
    if(!luaState)
    {
        autotestingLocalizationSystem->SetCurrentLocale(LocalizationSystem::Instance()->GetCurrentLocale());
        autotestingLocalizationSystem->InitWithDirectory("~res:/Autotesting/Strings/");
        
        bool isOk = true;
        luaState = lua_open();
        luaL_openlibs(luaState);
        String errors;
        
        if(isOk)
        {
            isOk = LoadWrappedLuaObjects();
        }
        else
        {
            errors += " luaL_openlibs failed";
        }
        
        if(isOk)
        {
            isOk = RunScriptFromFile("~res:/Autotesting/Scripts/autotesting_api.lua");
        }
        else
        {
            errors += ", LoadWrappedLuaObjects failed";
        }
        
        FilePath pathToAutotesting = "~res:/Autotesting/";
        String setPackagePathScript = Format("SetPackagePath(\"%s\")", pathToAutotesting.GetAbsolutePathname().c_str());
        if(isOk)
        {
            isOk = RunScript(setPackagePathScript);
        }
        else
        {
            errors += ", autotesting_api.lua failed";
        }
        
        if(isOk)
        {
            isOk = LoadScriptFromFile(luaFilePath);
        }
        else
        {
            errors += ", " + setPackagePathScript + " failed";
        }
        
        if(isOk)
        {
            AutotestingSystem::Instance()->OnInit();
        }
        else
        {
            errors += ", " + luaFilePath + " failed";
        }
        
        if(isOk)
        {
            AutotestingSystem::Instance()->RunTests();
        }
        else
        {
            AutotestingSystem::Instance()->OnError(errors);
        }
    }
}

void AutotestingSystemLua::StartTest()
{
    Logger::Debug("AutotestingSystemLua::StartTest");
    RunScript();
}
    
// Multiplayer API
void AutotestingSystemLua::WriteState(const String & device, const String & state)
{
	Logger::Debug("AutotestingSystemLua::WriteState device=%s state=%s", device.c_str(), state.c_str());
	AutotestingDB::Instance()->WriteState(device,state);
}

void AutotestingSystemLua::WriteCommand(const String & device, const String & state)
{
	Logger::Debug("AutotestingSystemLua::WriteCommand device=%s command=%s", device.c_str(), state.c_str());
	AutotestingDB::Instance()->WriteCommand(device,state);
}

String AutotestingSystemLua::ReadState(const String & device)
{
	Logger::Debug("AutotestingSystemLua::ReadState device=%s", device.c_str());
	return AutotestingDB::Instance()->ReadState(device);
}

String AutotestingSystemLua::ReadCommand(const String & device)
{
	Logger::Debug("AutotestingSystemLua::ReadCommand device=%s", device.c_str());
	return AutotestingDB::Instance()->ReadCommand(device);
}

void AutotestingSystemLua::InitializeDevice(const String & device)
{
	Logger::Debug("AutotestingSystemLua::InitializeDevice device=%s", device.c_str());
	AutotestingSystem::Instance()->InitializeDevice(device);
}


String AutotestingSystemLua::GetTestParameter(const String & device)
{
	Logger::Debug("AutotestingSystemLua::GetTestParameter device=%s", device.c_str());
	String result =  AutotestingDB::Instance()->GetStringTestParameter(AutotestingSystem::Instance()->deviceName, device);
	Logger::Debug("AutotestingSystemLua::GetTestParameter result=%s", result.c_str());
	return result;
}


void AutotestingSystemLua::Update(float32 timeElapsed)
{
    RunScript("ResumeTest()"); //TODO: time 
}
    
float32 AutotestingSystemLua::GetTimeElapsed()
{
    return SystemTimer::FrameDelta();
}
    
void AutotestingSystemLua::OnError(const String &errorMessage)
{
    Logger::Debug("AutotestingSystemLua::OnError %s", errorMessage.c_str());
    AutotestingSystem::Instance()->OnError(errorMessage);
}

void AutotestingSystemLua::OnTestStart(const String &testName)
{
	Logger::Debug("AutotestingSystemLua::OnTestStart %s", testName.c_str());
	AutotestingSystem::Instance()->OnTestStart(testName);
}

void AutotestingSystemLua::OnTestFinished()
{
    Logger::Debug("AutotestingSystemLua::OnTestFinished");
    AutotestingSystem::Instance()->OnTestsFinished();
}
    
void AutotestingSystemLua::OnStepStart(const String &stepName)
{
	Logger::Debug("AutotestingSystemLua::OnStepStart %s", stepName.c_str());
	AutotestingSystem::Instance()->OnStepStart(stepName);
}

void AutotestingSystemLua::Log(const String &level, const String &message)
{
	Logger::Debug("AutotestingSystemLua::Log [%s]%s", level.c_str(), message.c_str());
	AutotestingDB::Instance()->Log(level, message);
}

void AutotestingSystemLua::WriteString(const String & name, const String & text)
{
	Logger::Debug("AutotestingSystemLua::WriteString name=%s text=%s", name.c_str(), text.c_str());
	AutotestingDB::Instance()->WriteString(name, text);
}

String AutotestingSystemLua::ReadString(const String & name)
{
	Logger::Debug("AutotestingSystemLua::ReadString name=%s", name.c_str());
	return AutotestingDB::Instance()->ReadString(name);
}

bool AutotestingSystemLua::SaveKeyedArchiveToDB(const String &archiveName, KeyedArchive *archive, const String &docName)
{
	Logger::Debug("AutotestingSystemLua::SaveKeyedArchiveToDB");
	return AutotestingDB::Instance()->SaveKeyedArchiveToDB(archiveName, archive, docName);
}

String AutotestingSystemLua::MakeScreenshot()
{
	Logger::Debug("AutotestingSystemLua::MakeScreenshot");
	AutotestingSystem::Instance()->MakeScreenShot();
	return AutotestingSystem::Instance()->GetScreenShotName();
}

UIControl *AutotestingSystemLua::GetScreen()
{
    return UIControlSystem::Instance()->GetScreen();
}

UIControl *AutotestingSystemLua::FindControl(const String &path)
{
    Logger::Debug("AutotestingSystemLua::FindControl %s", path.c_str());
    
    Vector<String> controlPath;
    ParsePath(path, controlPath);
    
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

UIControl* AutotestingSystemLua::FindControl(UIControl* srcControl, const String &controlName)
{
	if(UIControlSystem::Instance()->GetLockInputCounter() > 0) return NULL;

	if(srcControl)
	{
		int32 index = atoi(controlName.c_str());
		if(Format("%d",index) != controlName)
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

UIControl* AutotestingSystemLua::FindControl(UIControl* srcControl, int32 index)
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

UIControl* AutotestingSystemLua::FindControl(UIList* srcList, int32 index)
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
				if(cell->GetIndex() == index && IsCenterInside(srcList, cell))
				{
					return cell;
				}
			}
		}
	}
	return NULL;
}

bool AutotestingSystemLua::IsCenterInside(UIControl* parent, UIControl* child)
{
	bool isInside = false;
	if(parent && child)
	{
		const Rect &parentRect = parent->GetGeometricData().GetUnrotatedRect();
		const Rect &childRect = child->GetGeometricData().GetUnrotatedRect();

		// check if child center is inside parent rect
		isInside = ((parentRect.x <= childRect.x + childRect.dx/2) && (childRect.x + childRect.dx/2 <= parentRect.x + parentRect.dx) &&
			(parentRect.y <= childRect.y + childRect.dy/2) && (childRect.y + childRect.dy/2 <= parentRect.y + parentRect.dy));

	}
	return isInside;
}    

bool AutotestingSystemLua::SetText(const String &path, const String &text)
{
    Logger::Debug("AutotestingSystemLua::SetText %s %s", path.c_str(), text.c_str());
    UITextField *tf = dynamic_cast<UITextField*>(FindControl(path));
    if(tf)
    {
        tf->SetText(StringToWString(text));
        return true;
    }
    return false;
}

void AutotestingSystemLua::KeyPress(int32 keyChar)
{
	UITextField *uiTextField = dynamic_cast<UITextField*>(UIControlSystem::Instance()->GetFocusedControl()); 
	if (uiTextField)
	{
		UIEvent keyPress;
		keyPress.tid = keyChar;
		keyPress.phase = UIEvent::PHASE_KEYCHAR;
		keyPress.tapCount = 1;
		keyPress.keyChar = keyChar;

		Logger::Debug("AutotestingSystemLua::KeyPress %d phase=%d count=%d point=(%f, %f) physPoint=(%f,%f) key=%c", keyPress.tid, keyPress.phase, keyPress.tapCount, keyPress.point.x, keyPress.point.y, keyPress.physPoint.x, keyPress.physPoint.y, keyPress.keyChar);

		if (keyPress.tid == DVKEY_BACKSPACE)
		{
			//TODO: act the same way on iPhone
			WideString str = L"";
			if(uiTextField->GetDelegate()->TextFieldKeyPressed(uiTextField, (int32)uiTextField->GetText().length(), -1, str))
			{
				uiTextField->SetText(uiTextField->GetAppliedChanges((int32)uiTextField->GetText().length(),  -1, str));
			}
			
		}
		else if (keyPress.tid == DVKEY_ENTER)
		{
			uiTextField->GetDelegate()->TextFieldShouldReturn(uiTextField);
		}
		else if (keyPress.tid == DVKEY_ESCAPE)
		{
			uiTextField->GetDelegate()->TextFieldShouldCancel(uiTextField);
		}
		else if(keyPress.keyChar != 0)
		{
			WideString str;
			str += keyPress.keyChar;
			if(uiTextField->GetDelegate()->TextFieldKeyPressed(uiTextField, (int32)uiTextField->GetText().length(), 1, str))
			{
				uiTextField->SetText(uiTextField->GetAppliedChanges((int32)uiTextField->GetText().length(),  1, str));
			}
		}
	}

	/*
	UIEvent keyPress;
	keyPress.tid = keyChar;
	keyPress.phase = UIEvent::PHASE_KEYCHAR;
	keyPress.tapCount = 1;
	keyPress.keyChar = keyChar;

	Logger::Debug("AutotestingSystemLua::KeyPress %d phase=%d count=%d point=(%f, %f) physPoint=(%f,%f) key=%c", keyPress.tid, keyPress.phase, keyPress.tapCount, keyPress.point.x, keyPress.point.y, keyPress.physPoint.x, keyPress.physPoint.y, keyPress.keyChar);

	Vector<UIEvent> emptyTouches;
	Vector<UIEvent> touches;
	touches.push_back(keyPress);
	UIControlSystem::Instance()->OnInput(0, emptyTouches, touches);
	AutotestingSystem::Instance()->OnInput(keyPress);
	*/
}

String AutotestingSystemLua::GetText(UIControl *control)
{
	Logger::Debug("AutotestingSystemLua::GetText =%s", control->GetName().c_str());
	UIStaticText *uiStaticText = dynamic_cast<UIStaticText*>(control);
	if(uiStaticText)
	{
		return WStringToString(uiStaticText->GetText());
	}
	UITextField *uiTextField = dynamic_cast<UITextField*>(control);
	if(uiTextField)
	{
		return WStringToString(uiTextField->GetText());
	}
	return "";
}

bool AutotestingSystemLua::CheckText(UIControl *control, const String &expectedText)
{
	UIStaticText *uiStaticText = dynamic_cast<UIStaticText*>(control);
	if(uiStaticText)
	{
		String actualText = WStringToString(uiStaticText->GetText());
		Log("DEBUG", Format("Compare text in control %s with expected text", uiStaticText->GetName().c_str()));
		Log("DEBUG", actualText);
		Log("DEBUG", expectedText);
		return (actualText == expectedText);
	}
	UITextField *uiTextField = dynamic_cast<UITextField*>(control);
	if(uiTextField)
	{
		String actualText = WStringToString(uiTextField->GetText());
		Log("DEBUG", Format("Compare text in control %s with expected text", uiTextField->GetName().c_str()));
		Log("DEBUG", actualText);
		Log("DEBUG", expectedText);
		return (actualText == expectedText);
	}
	return false;
}

bool AutotestingSystemLua::CheckMsgText(UIControl *control, const String &key)
{
	WideString expectedText = StringToWString(key);
	//TODO: check key in localized strings for Lua
	expectedText = autotestingLocalizationSystem->GetLocalizedString(expectedText);

	UIStaticText *uiStaticText = dynamic_cast<UIStaticText*>(control);
	if(uiStaticText)
	{
		WideString actualText = uiStaticText->GetText();
		Log("DEBUG", Format("Compare text in control %s with text by key %s", uiStaticText->GetName().c_str(), key.c_str()));
		Log("DEBUG", WStringToString(actualText));
		Log("DEBUG", WStringToString(expectedText));
		return (actualText == expectedText);
	}
	UITextField *uiTextField = dynamic_cast<UITextField*>(control);
	if(uiTextField)
	{
		WideString actualText = uiTextField->GetText();
		Log("DEBUG", Format("Compare text in control %s with text by key %s", uiTextField->GetName().c_str(), key.c_str()));
		Log("DEBUG", WStringToString(actualText));
		Log("DEBUG", WStringToString(expectedText));
		return (actualText == expectedText);
	}
	return false;
}
    
void AutotestingSystemLua::TouchDown(const Vector2 &point, int32 touchId)
{
    Logger::Debug("AutotestingSystemLua::TouchDown point=(%f,%f) touchId=%d", point.x, point.y, touchId);
      
    UIEvent touchDown;
    touchDown.phase = UIEvent::PHASE_BEGAN;
    touchDown.tid = touchId;
    touchDown.tapCount = 1;
    UIControlSystem::Instance()->RecalculatePointToPhysical(point, touchDown.physPoint);
    UIControlSystem::Instance()->RecalculatePointToVirtual(touchDown.physPoint, touchDown.point);
        
    ProcessInput(touchDown);
}
    
void AutotestingSystemLua::TouchMove(const Vector2 &point, int32 touchId)
{
    Logger::Debug("AutotestingSystemLua::TouchMove point=(%f,%f) touchId=%d", point.x, point.y, touchId);
    
    //Logger::Debug("TouchAction::TouchMove point=(%f, %f)", point.x, point.y);
    UIEvent touchMove;
    touchMove.tid = touchId;
    touchMove.tapCount = 1;
    UIControlSystem::Instance()->RecalculatePointToPhysical(point, touchMove.physPoint);
    UIControlSystem::Instance()->RecalculatePointToVirtual(touchMove.physPoint, touchMove.point);
    
    if(AutotestingSystem::Instance()->IsTouchDown(touchId))
    {
        touchMove.phase = UIEvent::PHASE_DRAG;
        ProcessInput(touchMove);
    }
    else
    {
#if defined(__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_ANDROID__)
        Logger::Warning("AutotestingSystemLua::TouchMove point=(%f, %f) ignored no touch down found", point.x, point.y);
#else
        touchMove.phase = UIEvent::PHASE_MOVE;
        ProcessInput(touchMove);
#endif
    }
}
    
void AutotestingSystemLua::TouchUp(int32 touchId)
{
    Logger::Debug("AutotestingSystemLua::TouchUp touchId=%d", touchId);
    UIEvent touchUp;
    if(!AutotestingSystem::Instance()->FindTouch(touchId, touchUp))
    {
        Logger::Error("TouchAction::TouchUp touch down not found");
    }
    touchUp.phase = UIEvent::PHASE_ENDED;
    touchUp.tid = touchId;
    
    ProcessInput(touchUp);
}

void AutotestingSystemLua::ProcessInput(const UIEvent &input)
{
	Logger::Debug("AutotestingSystemLua::ProcessInput %d phase=%d count=%d point=(%f, %f) physPoint=(%f,%f) key=%c",input.tid, input.phase, input.tapCount, input.point.x, input.point.y, input.physPoint.x, input.physPoint.y, input.keyChar);

	Vector<UIEvent> emptyTouches;
	Vector<UIEvent> touches;
	touches.push_back(input);
	UIControlSystem::Instance()->OnInput(0, emptyTouches, touches);

	AutotestingSystem::Instance()->OnInput(input);
}

void AutotestingSystemLua::ParsePath(const String &path, Vector<String> &parsedPath)
{
    Logger::Debug("AutotestingSystemLua::ParsePath path=%s", path.c_str());
    Split(path, "/", parsedPath);
}
    
bool AutotestingSystemLua::LoadWrappedLuaObjects()
{
    if(!luaState) return false; //TODO: report error?
    
	bool ret = true;
    luaopen_AutotestingSystem(luaState);	// load the wrappered module
    luaopen_UIControl(luaState);	// load the wrappered module
    luaopen_Rect(luaState);	// load the wrappered module
    luaopen_Vector(luaState);	// load the wrappered module
    luaopen_KeyedArchive(luaState);	// load the wrappered module

    if(delegate)
    {
        ret = delegate->LoadWrappedLuaObjects(luaState);
    }
    //TODO: check if modules really loaded
    return ret;
}

bool AutotestingSystemLua::LoadScript(const String &luaScript)
{
    if(!luaState) return false;
    
    if(luaL_loadstring(luaState, luaScript.c_str()) != 0)
    {
        Logger::Error("AutotestingSystemLua::LoadScript Error: unable to load %s", luaScript.c_str());
        return false;
    }
    return true;
}
    
bool AutotestingSystemLua::LoadScriptFromFile(const String &luaFilePath)
{
    if(!luaState) return false;
    
    String realPath = FilePath(luaFilePath).GetAbsolutePathname();
    if(luaL_loadfile(luaState, realPath.c_str()) != 0)
    {
        Logger::Error("AutotestingSystemLua::LoadScriptFromFile Error: unable to load %s", realPath.c_str());
        return false;
    }
    return true;
}
    
bool AutotestingSystemLua::RunScriptFromFile(const String &luaFilePath)
{
    Logger::Debug("AutotestingSystemLua::RunScriptFromFile %s", luaFilePath.c_str());
    if(LoadScriptFromFile(luaFilePath))
    {
        return RunScript();
    }
    return false;
}
    
bool AutotestingSystemLua::RunScript(const DAVA::String &luaScript)
{
    //Logger::Debug("AutotestingSystemLua::RunScript %s", luaScript.c_str());
    if(LoadScript(luaScript))
    {
        return RunScript();
    }
    return false;
}
    
bool AutotestingSystemLua::RunScript()
{
    //Logger::Debug("AutotestingSystemLua::RunScript");
    lua_pcall(luaState, 0, 0, 0); //TODO: LUA_MULTRET?
    //TODO: check if lua_pcall was successfull
    return true;
}

};

#endif //__DAVAENGINE_AUTOTESTING__