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
#ifndef __DAVAENGINE_AUTOTESTING_DB_H__
#define __DAVAENGINE_AUTOTESTING_DB_H__

#include "DAVAConfig.h"

#ifdef __DAVAENGINE_AUTOTESTING__

//#define AUTOTESTING_DB_HOST    "10.128.19.33"
#define AUTOTESTING_DB_HOST    "by2-buildmachine.wargaming.net"
//#define AUTOTESTING_DB_HOST    "10.128.128.5"
//#define AUTOTESTING_DB_HOST    "192.168.1.2"
#define AUTOTESTING_DB_PORT  27017
#define AUTOTESTING_DB_NAME  "Autotesting"

#include "Database/MongodbClient.h"

#include "Autotesting/MongodbUpdateObject.h"
#include "AutotestingSystem.h"

namespace DAVA
{
class Image;

class AutotestingDB : public Singleton<AutotestingDB>
{
public:
	AutotestingDB();
	~AutotestingDB();

	bool ConnectToDB(const String &name);
	

	// Work with log object in DB
	KeyedArchive *FindRunArchive(MongodbUpdateObject* dbUpdateObject, const String &auxArg);
	KeyedArchive *FindOrInsertRunArchive(MongodbUpdateObject* dbUpdateObject, const String &auxArg);

	KeyedArchive *InsertTestArchive(MongodbUpdateObject* dbUpdateObject, const String &testId, const String &testName, bool needClearGroup);
	KeyedArchive *InsertStepArchive(KeyedArchive *testArchive, const String &stepId, const String &description);

	KeyedArchive *FindOrInsertTestArchive(MongodbUpdateObject *dbUpdateObject, const String &testId);
	KeyedArchive *FindOrInsertTestStepArchive(KeyedArchive *testArchive, const String &stepId);
	KeyedArchive *FindOrInsertTestStepLogEntryArchive(KeyedArchive *testStepArchive, const String &logId);

	// Getting and Setting data from/in DB
	bool SaveToDB(MongodbUpdateObject *dbUpdateObject);

	void Log(const String &level, const String &message);

	String GetStringTestParameter(const String & deviceName, const String & parameter);
	int32 GetIntTestParameter(const String & deviceName, const String & parameter);

	String ReadString(const String & name);
	void WriteString(const String & name, const String & text);

	bool SaveKeyedArchiveToDB(const String &archiveName, KeyedArchive *archive, const String &docName);

	void UploadScreenshot(const String & name, Image *image);

	// multiplayer api
	void WriteState(const String & device, const String & state);
	void WriteCommand(const String & device, const String & state);

	String ReadState(const String & device);
	String ReadCommand(const String & device);


protected:
	MongodbClient *dbClient;


};


}

#endif //__DAVAENGINE_AUTOTESTING__

#endif //__DAVAENGINE_AUTOTESTING_DB_H__