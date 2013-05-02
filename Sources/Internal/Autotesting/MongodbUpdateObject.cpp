//
//  MongodbUpdateObject.cpp
//  Framework
//
//  Created by Dmitry Shpakov on 6/27/12.
//  Copyright (c) 2012 DAVA Consulting. All rights reserved.
//

#include "Autotesting/MongodbUpdateObject.h"

#ifdef __DAVAENGINE_AUTOTESTING__

#include "Database/MongodbClient.h"

namespace DAVA
{  
    
MongodbUpdateObject::MongodbUpdateObject()
    : MongodbObject()
    , updateObject(NULL)
    , updateData(new KeyedArchive())
{
}
    
MongodbUpdateObject::~MongodbUpdateObject()
{
    SafeRelease(updateObject);
    SafeRelease(updateData);
}
    
bool MongodbUpdateObject::LoadData()
{
    SafeRelease(updateData);
    updateData = new KeyedArchive();
    Finish();
    
    return MongodbClient::DBObjectToKeyedArchive(this, updateData);
}    
    
bool MongodbUpdateObject::SaveData()
{
    Logger::Debug("MongodbUpdateObject::SaveData");
    SafeRelease(updateObject);
    updateObject = new MongodbObject();
    updateObject->SetObjectName(GetObjectName());
    return MongodbClient::KeyedArchiveToDBObject(updateData, updateObject);
}    
    
KeyedArchive* MongodbUpdateObject::GetData()
{
    return updateData;
}
  
MongodbObject* MongodbUpdateObject::GetUpdateObject()
{
    return updateObject;
}

bool MongodbUpdateObject::SaveToDB(MongodbClient* dbClient)
{
    if(!updateObject)
    {
        if(!SaveData())
        {
            Logger::Error("MongodbUpdateObject::SaveToDB SaveData failed");
        }
    }
    
    if(dbClient && updateObject)
    {
        updateObject->Finish();
        
        //Logger::Debug("MongodbUpdateObject::SaveToDB old object:");
        //Print();
        
        //Logger::Debug("MongodbUpdateObject::SaveToDB new object:");
        //updateObject->Print();
        return dbClient->SaveObject(updateObject);
    }


	Logger::Error("MongodbUpdateObject::SaveToDB SaveData failed 2");
    return false;
}
    
};

#endif
