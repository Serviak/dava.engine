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

#include "DAVAEngine.h"
#include "Base/Singleton.h"
#include "FileSystem/FileSystem.h"
#include "Database/MongodbClient.h"

#include "Autotesting/MongodbUpdateObject.h"

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


namespace DAVA
{

class AutotestingSystem : public Singleton<AutotestingSystem>
{
public:
    AutotestingSystem();
    ~AutotestingSystem();

    void OnAppStarted();
    void OnAppFinished();

    void Update(float32 timeElapsed);
    void Draw();
    
    void Init(const String &_testName);
    inline bool IsInit() { return isInit; };
    
    void SetProjectName(const String &_projectName);
    
    void RunTests();
    
    void OnTestsSatrted();
    void OnTestAssert(const String & text, bool isPassed);
    void OnError(const String & errorMessage = "");
	void OnMessage(const String & logMessage = "");
    void OnTestsFinished();
    
    // helpers
    void OnInput(const UIEvent &input);
    
    inline Vector2 GetMousePosition() { return mouseMove.point; };
    bool FindTouch(int32 id, UIEvent &touch);
    bool IsTouchDown(int32 id);

    // DB Master-Helper relations
    void InitMultiplayer();
    void RegisterMasterInDB(int32 helpersCount);
    void RegisterHelperInDB();
    
protected:
    //DB
    bool ConnectToDB();
    void AddTestResult(const String &text, bool isPassed);
    void SaveTestToDB();
    
    String ReadMasterIDFromDB(); //TODO: get first available master
    
    bool CheckMasterHelpersReadyDB();
    //
    
    void ExitApp();
    
    bool isInit;
    bool isRunning;
    bool needExitApp;
    float32 timeBeforeExit;
    
    String projectName;
    String groupName;
    uint32 testsId;
    uint32 testsDate;
    int32 testIndex;

    String testName;
    String testFileName;
    String testFilePath;
    Vector< std::pair<String, bool> > testResults;

    MongodbClient *dbClient;
    bool isDB;
    bool needClearDB;
    
    bool isMaster;
    int32 requestedHelpers;
    String masterId; // for communication
    int32 masterRunId;
    bool isRegistered;
    bool isWaiting;
    bool isInitMultiplayer;
    String multiplayerName;
    float32 waitTimeLeft;

    String testReportsFolder;
    File* reportFile;

    Map<int32, UIEvent> touches;
    UIEvent mouseMove;
};

};

#endif //__DAVAENGINE_AUTOTESTING__

#endif //__DAVAENGINE_AUTOTESTING_SYSTEM_H__