/*==================================================================================
    Copyright (c) 2008, DAVA, INC
    All rights reserved.

    Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the DAVA, INC nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE DAVA CONSULTING, LLC AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL DAVA CONSULTING, LLC BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/

#include "Base/IntrospectionBase.h"
#include "Base/Meta.h"

namespace DAVA
{

IntrospectionDesription::IntrospectionDesription(const char *_text)
	: text(_text)
{}

IntrospectionDesription::IntrospectionDesription(const char *_text, const EnumMap* _enumMap)
	: text(_text)
	, enumMap(_enumMap)
{ }

IntrospectionMember::IntrospectionMember(const char *_name, const char *_desc, const int _offset, const MetaInfo *_type, int _flags /* = 0 */)
	: name(_name), desc(_desc), offset(_offset), type(_type), flags(_flags)
{ }

const char* IntrospectionMember::Name() const
{
	return name;
}

const char* IntrospectionMember::Desc() const
{
	return desc;
}

const MetaInfo* IntrospectionMember::Type() const
{
	return type;
}

void* IntrospectionMember::Pointer(void *object) const
{
	return (((char *) object) + offset);
}

void* IntrospectionMember::Data(void *object) const
{
	if(type->IsPointer())
	{
		return *(void **) Pointer(object);
	}
	else
	{
		return Pointer(object);
	}
}

VariantType IntrospectionMember::Value(void *object) const
{
	return VariantType::LoadData(Pointer(object), type);
}

void IntrospectionMember::SetValue(void *object, const VariantType &val) const
{
	VariantType::SaveData(Pointer(object), type, val);
}

const IntrospectionCollection* IntrospectionMember::Collection() const
{
	return NULL;
}

int IntrospectionMember::Flags() const
{
	return flags;
}

};
