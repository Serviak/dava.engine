/*==================================================================================
	Copyright (c) 2008, DAVA, INC
	All rights reserved.

	Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
	* Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
	* Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
	* Neither the name of the DAVA, INC nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

	THIS SOFTWARE IS PROVIDED BY THE DAVA, INC AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
	WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL DAVA, INC BE LIABLE FOR ANY
	DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
	LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
	(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
	=====================================================================================*/
#include "Autotesting/AutotestingDB.h"

#ifdef __DAVAENGINE_AUTOTESTING__
#include "Platform/DeviceInfo.h"


// Work with MongoDb API
#define AUTOTESTING_TESTS "Tests"
#define AUTOTESTING_STEPS "Steps"
#define AUTOTESTING_LOG "Log"

namespace DAVA
{
	AutotestingDB::AutotestingDB()
		: dbClient(NULL)
		, logFilePath(FilePath(""))
		, logsFolder(FilePath(""))
		, autoSys(NULL)
	{
		autoSys = AutotestingSystem::Instance();
	}

	AutotestingDB::~AutotestingDB()
	{
		CloseConnection();
	}

	bool AutotestingDB::ConnectToDB(const String &collection, const String &dbName, const String &dbHost, const int32 dbPort)
	{
		DVASSERT(NULL == dbClient);

		dbClient = MongodbClient::Create(dbHost, dbPort);
		if (dbClient)
		{
			dbClient->SetDatabaseName(dbName);
			dbClient->SetCollectionName(collection);
		}
		return (NULL != dbClient);
	}

	void AutotestingDB::CloseConnection()
	{
		if (dbClient)
		{
			dbClient->Disconnect();
			SafeRelease(dbClient);
		}
	}

	String AutotestingDB::GetStringTestParameter(const String &deviceName, const String &parameter)
	{
		Logger::Info("AutotestingDB::GetStringTestParameter deviceName=%s, parameter=%s", deviceName.c_str(), parameter.c_str());
		MongodbUpdateObject *dbUpdateObject = new MongodbUpdateObject();
		KeyedArchive *currentRunArchive = FindBuildArchive(dbUpdateObject, "autotesting_system");
		KeyedArchive *deviceArchive = currentRunArchive->GetArchive(deviceName.c_str());
		if (!deviceArchive)
		{
			autoSys->ForceQuit(Format("Couldn't find archive for %s device", deviceName.c_str()));
		}
		String result = deviceArchive->GetString(parameter.c_str(), "not_found");
		SafeRelease(dbUpdateObject);
		Logger::Info("AutotestingDB::GetStringTestParameter return deviceName=%s value: %s", deviceName.c_str(), result.c_str());
		return result;
	}

	int32 AutotestingDB::GetIntTestParameter(const String &deviceName, const String &parameter)
	{
		Logger::Info("AutotestingDB::GetIntTestParameter deviceName=%s, parameter=%s", deviceName.c_str(), parameter.c_str());
		MongodbUpdateObject *dbUpdateObject = new MongodbUpdateObject();
		KeyedArchive *currentRunArchive = FindBuildArchive(dbUpdateObject, "autotesting_system");
		KeyedArchive *deviceArchive = currentRunArchive->GetArchive(deviceName.c_str());
		if (!deviceArchive)
		{
			autoSys->ForceQuit(Format("Couldn't find archive for %s device", deviceName.c_str()));
		}
		int32 result = deviceArchive->GetInt32(parameter.c_str(), -9999);
		SafeRelease(dbUpdateObject);
		Logger::Info("AutotestingDB::GetIntTestParameter return deviceName=%s value: %d", deviceName.c_str(), result);
		return result;
	}

	// BUILD Level
	KeyedArchive *AutotestingDB::FindBuildArchive(MongodbUpdateObject *dbUpdateObject, const String &auxArg)
	{
		if (auxArg.length() == 0)
		{
			autoSys->ForceQuit("Archive name is empty.");
		}
		String archiveName = Format("%s", auxArg.c_str());
		if (!dbClient->FindObjectByKey(archiveName, dbUpdateObject))
		{
			autoSys->ForceQuit(Format("Couldn't find '%s' archive.", archiveName.c_str()));
		}
		dbUpdateObject->LoadData();
		return dbUpdateObject->GetData();
	}

	KeyedArchive *AutotestingDB::FindOrInsertBuildArchive(MongodbUpdateObject *dbUpdateObject, const String &auxArg)
	{
		String testsName;

		if (auxArg.length() != 0)
		{
			testsName = Format("%s", auxArg.c_str());
		}
		else
		{
			testsName = Format("%s_%s_%s", autoSys->buildDate.c_str(), autoSys->buildId.c_str(), autoSys->deviceName.c_str());
		}

		bool isFound = dbClient->FindObjectByKey(testsName, dbUpdateObject);

		if (!isFound)
		{
			dbUpdateObject->SetObjectName(testsName);
			dbUpdateObject->AddString("Platform", DeviceInfo::GetPlatformString());
			dbUpdateObject->AddString("Date", autoSys->buildDate.c_str());
			dbUpdateObject->AddString("RunId", autoSys->runId.c_str());
			dbUpdateObject->AddString("Device", autoSys->deviceName.c_str());
			dbUpdateObject->AddString("BuildId", autoSys->buildId.c_str());
			dbUpdateObject->AddString("Branch", autoSys->branch.c_str());
			dbUpdateObject->AddString("BranchRevision", autoSys->branchRev.c_str());
			dbUpdateObject->AddString("Framework", autoSys->framework.c_str());
			dbUpdateObject->AddString("FrameworkRevision", autoSys->frameworkRev.c_str());
			// TODO: After realization GetOsVersion() DF-3940
			dbUpdateObject->AddString("OSVersion", DeviceInfo::GetVersion());
			dbUpdateObject->AddString("Model", DeviceInfo::GetModel());
			Logger::Debug("AutotestingSystem::InsertTestArchive new MongodbUpdateObject %s", testsName.c_str());
		}

		dbUpdateObject->LoadData();
		KeyedArchive *dbUpdateData = dbUpdateObject->GetData();

		return dbUpdateData;
	}

	void AutotestingDB::WriteLogHeader()
	{
#if defined(__DAVAENGINE_ANDROID__)
		logsFolder = FileSystem::Instance()->GetPublicDocumentsPath() + "/autoLogs";
#else
		logsFolder = FileSystem::Instance()->GetCurrentDocumentsDirectory() + "/autoLogs";
#endif //#if defined(__DAVAENGINE_ANDROID__)
		Logger::Info("AutotestingSystem::AutotestingDB path to log file: %s", logsFolder.GetStringValue().c_str());
		if (!FileSystem::Instance()->IsDirectory(logsFolder))
		{
			FileSystem::Instance()->CreateDirectory(logsFolder);
		}
        autoSys->testIndex++;
        logFilePath = logsFolder + Format("/%s:%s:%s:%d.log", autoSys->groupName.c_str(), autoSys->testFileName.c_str(), autoSys->runId.c_str(), autoSys->testIndex);
		if (FileSystem::Instance()->IsFile(logFilePath))
		{
			FileSystem::Instance()->DeleteFile(logFilePath);
		}
		String message = Format("Platform:%s\nName:%s\nModel:%s\nOSVersion:%s\n", AutotestingSystemLua::Instance()->GetPlatform().c_str(),
			autoSys->deviceName.c_str(), DeviceInfo::GetModel().c_str(), DeviceInfo::GetVersion().c_str());
		WriteLog(message.c_str());
		DateTime time = DateTime::Now();
        //Get time.GetMonth() return month number - 1. Ex for 01(Jan) it return 00(Jan).
		String currentDay = Format("%d-%d-%d", time.GetYear(), time.GetMonth() + 1, time.GetDay());
		message = Format("BuildDate:%s\nLaunchDate:%s\nRunId:%s\nBuildId:%s\n", autoSys->buildDate.c_str(), currentDay.c_str(), autoSys->runId.c_str(), autoSys->buildId.c_str());
		WriteLog(message.c_str());
		message = Format("Client:%s\nClientRevision:%s\nFramework:%s\nFrameworkRevision:%s\n", autoSys->branch.c_str(), autoSys->branchRev.c_str(),
			autoSys->framework.c_str(), autoSys->frameworkRev.c_str());
        WriteLog(message.c_str());
        message = Format("TestGroup:%s\nFileName:%s\n", autoSys->groupName.c_str(), autoSys->testFileName.c_str());
		WriteLog(message.c_str());
	}
    
	void AutotestingDB::WriteLog(const char8 *text, ...)
	{
		if (!text || text[0] == '\0') return;
		File *file = File::Create(logFilePath, File::APPEND | File::WRITE);
		if (!file)
		{
			Logger::Error("Can't open/create log file.");
			return;
		}
		va_list vl;
		va_start(vl, text);
		char tmp[4096] = { 0 };
		vsnprintf(tmp, sizeof(tmp) - 2, text, vl);
		file->Write(text, sizeof(char) *strlen(tmp));
		file->Release();
		va_end(vl);
	}

    void AutotestingDB::Log(const String &level, const String &message)
	{
		String textLog = Format("[%s:%s] %s\n", autoSys->GetCurrentTimeString().c_str(), level.c_str(), message.c_str());
		WriteLog(textLog.c_str());
	}

	// DEPRECATED: Rewrite for new DB conception
	String AutotestingDB::ReadCommand(const String &device)
	{
		Logger::Info("AutotestingDB::ReadCommand device=%s", device.c_str());

		MongodbUpdateObject *dbUpdateObject = new MongodbUpdateObject();
		KeyedArchive *currentRunArchive = FindOrInsertBuildArchive(dbUpdateObject, "_multiplayer");

		String result;
		result = currentRunArchive->GetString(device + "_command", "not_found");

		SafeRelease(dbUpdateObject);

		Logger::Info("AutotestingDB::ReadCommand device=%s: '%s'", device.c_str(), result.c_str());
		return result;
	}

	// DEPRECATED: Rewrite for new DB conception
	String AutotestingDB::ReadState(const String &device)
	{
		Logger::Info("AutotestingDB::ReadState device=%s", device.c_str());


		MongodbUpdateObject *dbUpdateObject = new MongodbUpdateObject();
		KeyedArchive *currentRunArchive = FindOrInsertBuildArchive(dbUpdateObject, "_multiplayer");
		String result;

		result = currentRunArchive->GetString(device, "not_found");
		SafeRelease(dbUpdateObject);
		Logger::Info("AutotestingDB::ReadState device=%s: '%s'", device.c_str(), result.c_str());
		return result;
	}

	// DEPRECATED: Rewrite for new DB conception
	String AutotestingDB::ReadString(const String &name)
	{
		Logger::Info("AutotestingSystem::ReadString name=%s", name.c_str());

		MongodbUpdateObject *dbUpdateObject = new MongodbUpdateObject();
		KeyedArchive *currentRunArchive = FindOrInsertBuildArchive(dbUpdateObject, "_aux");
		String result;

		result = currentRunArchive->GetString(name, "not_found");

		SafeRelease(dbUpdateObject);
		Logger::Info("AutotestingSystem::ReadString name=%name: '%s'", name.c_str(), result.c_str());
		return result;
	}

	bool AutotestingDB::SaveKeyedArchiveToDevice(const String &archiveName, KeyedArchive *archive)
	{
		String fileName = Format("/%s:%s:%s:%d:%s.yaml", autoSys->groupName.c_str(), autoSys->testFileName.c_str(), autoSys->runId.c_str(), autoSys->testIndex, archiveName.c_str());
        Logger::Info("AutotestingDB::Save keyed archive '%s' to device.", fileName.c_str());
		return archive->SaveToYamlFile(logsFolder + fileName);
	}

	bool AutotestingDB::SaveToDB(MongodbUpdateObject *dbUpdateObject)
	{
		uint64 startTime = SystemTimer::Instance()->AbsoluteMS();
		Logger::Info("AutotestingSystem::SaveToDB");

		bool ret = dbUpdateObject->SaveToDB(dbClient);

		if (!ret)
		{
			Logger::Error("AutotestingSystem::SaveToDB failed");
		}

		uint64 finishTime = SystemTimer::Instance()->AbsoluteMS();
		Logger::Info("AutotestingSystem::SaveToDB FINISH result time %d", finishTime - startTime);
		return ret;
	}

	void AutotestingDB::UploadScreenshot(const String &name, Image *image)
	{
		if (dbClient)
		{
            dbClient->SaveBufferToGridFS(Format("%s_%dx%d", name.c_str(), image->GetWidth(), image->GetHeight()),
                reinterpret_cast<char*>(image->GetData()), image->dataSize);
		}
	}

	// DEPRECATED: Rewrite for new DB conception
	void AutotestingDB::WriteCommand(const String &device, const String &command)
	{
		Logger::Info("AutotestingDB::WriteCommand device=%s command=%s", device.c_str(), command.c_str());

		MongodbUpdateObject *dbUpdateObject = new MongodbUpdateObject();
		KeyedArchive *currentRunArchive = FindOrInsertBuildArchive(dbUpdateObject, "_multiplayer");

		currentRunArchive->SetString(device + "_command", command);

		SaveToDB(dbUpdateObject);
		SafeRelease(dbUpdateObject);
	}

	// DEPRECATED: Rewrite for new DB conception
	void AutotestingDB::WriteState(const String &device, const String &state)
	{
		Logger::Info("AutotestingDB::WriteState device=%s state=%s", device.c_str(), state.c_str());

		MongodbUpdateObject *dbUpdateObject = new MongodbUpdateObject();
		KeyedArchive *currentRunArchive = FindOrInsertBuildArchive(dbUpdateObject, "_multiplayer");

		currentRunArchive->SetString(device, state);

		SaveToDB(dbUpdateObject);
		SafeRelease(dbUpdateObject);
	}

	// DEPRECATED: Rewrite for new DB conception
	void AutotestingDB::WriteString(const String &name, const String &text)
	{
		Logger::Info("AutotestingSystem::WriteString name=%s text=%s", name.c_str(), text.c_str());

		MongodbUpdateObject *dbUpdateObject = new MongodbUpdateObject();
		KeyedArchive *currentRunArchive = FindOrInsertBuildArchive(dbUpdateObject, "_aux");

		currentRunArchive->SetString(name, text);

		SaveToDB(dbUpdateObject);
		SafeRelease(dbUpdateObject);

		Logger::Info("AutotestingSystem::WriteString finish");
	}

	// auxiliary methods
	void AutotestingDB::SetTestStarted()
	{
		Logger::Info("AutotestingSystem::SetTestStarted for test: %s", autoSys->testFileName.c_str());
		MongodbUpdateObject *dbUpdateObject = new MongodbUpdateObject();
		KeyedArchive *currentRunArchive = FindBuildArchive(dbUpdateObject, "autotesting_system");
		if (!currentRunArchive)
		{
			autoSys->ForceQuit(Format("Couldn't find archive autotesting_system device"));
		}
		KeyedArchive *deviceArchive = currentRunArchive->GetArchive(autoSys->deviceName.c_str(), NULL);

		if (!deviceArchive)
		{
			autoSys->ForceQuit(Format("Couldn't find archive for %s device", autoSys->deviceName.c_str()));
		}
		deviceArchive->SetString("Started", "1");
		deviceArchive->SetString("BuildId", autoSys->buildId.c_str());
		deviceArchive->SetString("Framework", autoSys->framework.c_str());
		deviceArchive->SetString("Branch", autoSys->branch.c_str());
        deviceArchive->SetInt32("TestIndex", autoSys->testIndex);
		SaveToDB(dbUpdateObject);
		SafeRelease(dbUpdateObject);
	}
}

#endif //__DAVAENGINE_AUTOTESTING__