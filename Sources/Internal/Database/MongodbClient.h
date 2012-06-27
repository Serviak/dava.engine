/*==================================================================================
    Copyright (c) 2008, DAVA Consulting, LLC
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
        * Created by Vitaliy Borodovsky 
=====================================================================================*/
#ifndef __DAVAENGINE_MONGODB_CLIENT_H__
#define __DAVAENGINE_MONGODB_CLIENT_H__

#include "Base/BaseTypes.h"
#include "Base/BaseObject.h"

namespace DAVA 
{
    
/**
	\defgroup Mongodb 
 */

/** 
	\ingroup Mongodb
	\brief this class is mongodb client and it used if you want to work with mongodb
 */

class MongodbObject;
class MongodbClientInternalData;
class MongodbClient: public BaseObject
{
protected:
	MongodbClient();
	
public:
	virtual ~MongodbClient();

	static MongodbClient *Create(const String &ip, int32 port);
	
    bool Connect(const String &ip, int32 port);
    void Disconnect();
    
    void SetDatabaseName(const String &newDatabase);
    void SetCollectionName(const String &newCollection);

    void DropDatabase();
    void DropCollection();
    
    bool IsConnected();

    MongodbObject * FindObjectByKey(const String &key);
    bool FindObjectByKey(const String &key, MongodbObject *foundObject);
    MongodbObject * CreateObject();
    void DestroyObject(MongodbObject *object);

    bool SaveObject(MongodbObject *object);
    bool SaveObject(MongodbObject *newObject, MongodbObject *oldObject);

    
    bool SaveBinary(const String &key, uint8 *data, int32 dataSize);
    int32 GetBinarySize(const String &key);
    bool GetBinary(const String &key, uint8 *outData, int32 dataSize);

    void DumpDB();
protected:

    void LogError(const String functionName, int32 errorCode);

protected:
    
	MongodbClientInternalData *clientData;
    String database;
    String collection;
    String namespaceName;

    Vector<MongodbObject *> objects;
};

};

#endif // __DAVAENGINE_MONGODB_CLIENT_H__

