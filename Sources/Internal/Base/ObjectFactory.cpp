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
#include "Base/ObjectFactory.h"

namespace DAVA 
{	
ObjectFactory::ObjectFactory()
    : unregisteredClassName("<Unknown class>")
{
    
}

ObjectRegistrator::ObjectRegistrator(const String & name, CreateObjectFunc func, const std::type_info & info, uint32 size)
{
    ObjectFactory::Instance()->RegisterObjectCreator(name, func, info, size);	
}
    
ObjectRegistrator::ObjectRegistrator(const String & name, CreateObjectFunc func, const std::type_info & info, uint32 size, const String & alias)
{
    ObjectFactory::Instance()->RegisterObjectCreator(name, func, info, size, alias);	
}

void ObjectFactory::RegisterObjectCreator(const String & name, CreateObjectFunc func, const std::type_info & info, uint32 size)
{
	creatorMap[name] = func;
    nameMap[info.name()] = name;
    sizeMap[name] = size;
}

void ObjectFactory::RegisterObjectCreator(const String & name, CreateObjectFunc func, const std::type_info & info, uint32 size, const String & alias)
{
    creatorMap[name] = func;
    nameMap[info.name()] = alias;
    sizeMap[name] = size;
}
	
BaseObject * ObjectFactory::New(const String & name)
{
	Map<String, CreateObjectFunc>::iterator it = creatorMap.find(name);
	if (it != creatorMap.end())
	{
		CreateObjectFunc newFunc = it->second;
		return (newFunc)();
	}
	return 0;
}
    
void ObjectFactory::Dump()
{
    Map<String, CreateObjectFunc>::iterator it = creatorMap.begin();
    for (; it != creatorMap.end(); ++it)
    {
        Logger::Debug("Class: %s size: %d", it->first.c_str(), sizeMap[it->first]);
    }
}
}
