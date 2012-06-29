/*==================================================================================
    Copyright (c) 2012, DAVA Consulting, LLC
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the DAVA Consulting, LLC nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE DAVA CONSULTING, LLC AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL DAVA CONSULTING, LLC BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

    Revision History:
        * Created by Dmitry Shpakov 
=====================================================================================*/
#ifndef __DAVAENGINE_AUTOTESTING_SYSTEM_H__
#define __DAVAENGINE_AUTOTESTING_SYSTEM_H__

#include "DAVAConfig.h"

#ifdef __DAVAENGINE_AUTOTESTING__

#include "Autotesting/Action.h"
#include "Autotesting/TouchAction.h"
#include "Autotesting/AssertAction.h"

#include "Autotesting/MongodbUpdateObject.h"

#include "Base/Singleton.h"

#define __DAVAENGINE_AUTOTESTING_FILE__

#ifdef __DAVAENGINE_AUTOTESTING_FILE__
#include "FileSystem/FileSystem.h"
#endif

#define AUTOTESTING_DB_IP    "10.128.128.131"
#define AUTOTESTING_DB_PORT  27017
#define AUTOTESTING_DB_NAME  "Autotesting"

#if defined (__DAVAENGINE_MACOS__)
#define AUTOTESTING_PLATFORM_NAME  "MacOS"
#elif defined (__DAVAENGINE_IPHONE__)
#define AUTOTESTING_PLATFORM_NAME  "iOS"
#elif defined (__DAVAENGINE_WIN32__)
#define AUTOTESTING_PLATFORM_NAME  "Windows"
#elif defined (__DAVAENGINE_ANDROID__)
#define AUTOTESTING_PLATFORM_NAME  "Android"
#else
#define AUTOTESTING_PLATFORM_NAME  "Unknown"
#endif //PLATFORMS    

#include "Database/MongodbClient.h"


namespace DAVA
{

class AutotestingSystem : public Singleton<AutotestingSystem>
{
public:
    AutotestingSystem();
    ~AutotestingSystem();

    void OnAppStarted();
    void OnAppFinished();

    void Init(const String & _testName);
    
    void SetProjectName(const String &_projectName);

    void AddAction(Action* action);
    void AddActionsFromYaml(const String &yamlFilePath);

    void RunTests();

    void Update(float32 timeElapsed);
    void Draw();

    void OnTestsSatrted();
    void OnTestAssert(const String & text, bool isPassed);
    void OnError(const String & errorMessage = "");
    void OnTestsFinished();

    // API (high level)
    void Click(const Vector2 &point, int32 id = 1);
    void Click(const String &controlName, int32 id = 1);
    void Click(const Vector<String> &controlPath, int32 id = 1);

    void TouchDown(const Vector2 &point, int32 id = 1);
    void TouchDown(const String &controlName, int32 id = 1);
    void TouchDown(const Vector<String> &controlPath, int32 id = 1);

    void TouchUp(int32 id = 1);

    void TouchMove(const Vector2 &point, float32 time, int32 id = 1);
    void TouchMove(const String &controlName, float32 time, int32 id = 1);
    void TouchMove(const Vector<String> &controlPath, float32 time, int32 id = 1);

    void KeyPress(char16 keyChar);
    void KeyboardInput(const WideString &text);

    void SetText(const String &controlName, const WideString &text);
    void SetText(const Vector<String> &controlPath, const WideString &text);
    
    void Wait(float32 time);

    void WaitForUI(const String &controlName, float32 timeout = 10.0f);
    void WaitForUI(const Vector<String> &controlPath, float32 timeout = 10.0f);

    void Scroll(const String &controlName, int32 id = 1, float32 timeout = 10.0f);
    void Scroll(const Vector<String> &controlPath, int32 id = 1, float32 timeout = 10.0f);

    void AssertText(const WideString &expected, const Vector<String> &controlPath, const String &assertMessage = "");
    void AssertText(const Vector<String> &expectedControlPath, const Vector<String> &actualControlPath, const String &assertMessage = "");
    
    void AssertBool(bool expected, const Vector<String> &controlPath, const String &assertMessage = "");
    void AssertBool(const Vector<String> &expectedControlPath, const Vector<String> &actualControlPath, const String &assertMessage = "");    

    // helpers
    void OnInput(const UIEvent &input);

    inline Vector2 GetMousePosition() { return mouseMove.point; };
    bool FindTouch(int32 id, UIEvent &touch);
    bool IsTouchDown(int32 id);

protected:
    void AddActionsFromYamlNode(YamlNode* actionsNode);
    Vector<String> ParseControlPath(YamlNode* controlPathNode);

    void ExitApp();
    
    //DB
    bool ConnectToDB();
    void AddTestResult(const String &text, bool isPassed);
    void SaveTestToDB();
    //

    bool isInit;
    bool isRunning;

    Action* currentAction;
    Deque<Action*> actions;

    String projectName;
    uint32 testsId;
    uint32 testsDate;
    int32 testIndex;
    
    String testName;
    String testFileName;
    String testFilePath;
    Vector< std::pair<String, bool> > testResults;
    
    MongodbClient *dbClient;
    
    
#ifdef __DAVAENGINE_AUTOTESTING_FILE__
    String testReportsFolder;
    File* reportFile;
#endif
        
    // TODO: data, shared between tests
    // keep in shared object instead of AutotestingSystem
    Map<int32, UIEvent> touches;
    UIEvent mouseMove;
};

};

#endif //__DAVAENGINE_AUTOTESTING__

#endif //__DAVAENGINE_AUTOTESTING_SYSTEM_H__