/*
	DAVA SDK
	Introspection
	Author: Sergey Zdanevich
*/

#ifndef __DAVAENGINE_INTROSPECTION_H__
#define __DAVAENGINE_INTROSPECTION_H__

#include "Debug/DVAssert.h"
#include "FileSystem/VariantType.h"

#include "Base/Meta.h"
#include "Base/IntrospectionBase.h"
#include "Base/IntrospectionProperty.h"
#include "Base/IntrospectionCollection.h"
#include "Base/IntrospectionFlags.h"

namespace DAVA
{
	// ����� ������������. 
	// ��������� ������������ � ������ ����������������� ������. ������������ ����������� � ���������� � ��������� ����������,
	// �� �������� �� ���������� �������� ������.
	//
	// �������������:
	//
	// 	class A
	// 	{
	// 	public:
	// 		int		i;
	// 		String	s;
	// 		Matrix4	m;
	//		Vector<int> v;
	//
	//		String GetName() { return s; }
	//      void SetName(const String &_s) { s = _s; }
	// 
	// 		INTROSPECTION(A,
	// 			MEMBER(i, "simple int var", 0)
	// 			MEMBER(s, "string", 0)
	//			PROPERTY(s, "property with setter and getter", GetName, SetName, 0)
	//			COLLECTION(v, "vector collection")
	// 			);
	// 	};
	//
	//  class B : public A
	//	{
	//	public:
	//		int b;
	//	
	//		INTROSPECTION_EXTEND(B, A, 
	//			MEMBER(b)
	//			);
	//	}
	//
	class IntrospectionInfo
	{
	public:
		IntrospectionInfo(const char *_name, const IntrospectionMember **_members, const int _members_count)
			: name(_name)
			, meta(NULL)
			, base_info(NULL)
			, members(_members)
			, members_count(_members_count)
		{
			MembersInit();
		}

		IntrospectionInfo(const IntrospectionInfo *_base, const char *_name, const IntrospectionMember **_members, const int _members_count)
			: name(_name)
			, meta(NULL)
			, base_info(_base)
			, members(_members)
			, members_count(_members_count)
		{
			MembersInit();
		}

		~IntrospectionInfo()
		{
			MembersRelease();
		}

		const char* Name() const
		{
			return name;
		}

		const MetaInfo* Type() const
		{
			return meta;
		}

		int MembersCount() const
		{
			return members_count;
		}

		// ���������� ��������� �� ���� ������������ �� ��������� �������, ��� NULL ���� ����� �� ������.
		const IntrospectionMember* Member(int index) const
		{
			const IntrospectionMember *member = NULL;

			if(index < members_count)
				if(NULL != members[index])
					member = members[index];

			return member;
		}

		// ���������� ��������� �� ���� ������������ �� ��������� �����, ��� NULL ���� ����� �� ������.
		const IntrospectionMember* Member(const char* name) const
		{
			const IntrospectionMember *member = NULL;

			for(int i = 0; i < members_count; ++i)
			{
				if(NULL != members[i])
				{
					if(members[i]->name == name)
					{
						member = members[i];
						break;
					}
				}
			}

			return member;
		}

		// ���������� ��������� �� ������� ������������, ��� NULL ���� ����� �� ����������.
		const IntrospectionInfo* BaseInfo() const
		{
			return base_info;
		}
        
		// �������� ���������� � ������� ������������ ��� ���� � ��������� 
		// �� ����-���������� ���� T
        template<typename T>
        void OneTimeMetaSafeSet()
        {
            if(!metaOneTimeSet)
            {
                metaOneTimeSet = true;
                meta = MetaInfo::Instance<T>();
            }
        }
        
	protected:
		const char* name;
		const MetaInfo* meta;

		const IntrospectionInfo *base_info;
		const IntrospectionMember **members;
		int members_count;
        
        bool metaOneTimeSet;

		// ������������� ������ ������������
		// ��� ����� ������������ ������ ���� �������(�������), � ��������� ������
		// ������ ������������ ����� ������
		void MembersInit()
		{
			// ��������� ��� ��� ����� ������������ �������
			for(int i = 0; i < members_count; ++i)
			{
				// ���� ���� ���� �� ������, �� ����������� ��� ���������.
				if(NULL == members[i])
				{
					MembersRelease();
					break;
				}
			}
		}

		// ����������� ����� ������������ � ������������� �� ���������� � 0
		void MembersRelease()
		{
			for(int i = 0; i < members_count; ++i)
			{
				if(NULL != members[i])
				{
					delete members[i];
					members[i] = NULL;
				}
			}
			members_count = 0;
		}
	};
};


// ����������� ����������� ������ ������. ��. ������ � �������� ������ IntrospectionInfo
#define INTROSPECTION(_type, _members) \
	static const DAVA::IntrospectionInfo* TypeInfo() \
	{ \
		typedef _type ObjectT; \
		static const DAVA::IntrospectionMember* data[] = { _members }; \
		static DAVA::IntrospectionInfo info = DAVA::IntrospectionInfo(#_type, data, sizeof(data)/sizeof(data[0])); \
		info.OneTimeMetaSafeSet<_type>(); \
        return &info; \
	} \
	virtual const DAVA::IntrospectionInfo* GetTypeInfo() const \
	{ \
		return _type::TypeInfo(); \
	}

// ������������ �����������. ��. ������ � �������� ������ IntrospectionInfo
#define  INTROSPECTION_EXTEND(_type, _base_type, _members) \
	static const DAVA::IntrospectionInfo* TypeInfo() \
	{ \
		typedef _type ObjectT; \
		static const DAVA::IntrospectionMember* data[] = { _members }; \
		static DAVA::IntrospectionInfo info = DAVA::IntrospectionInfo(_base_type::TypeInfo(), #_type, data, sizeof(data)/sizeof(data[0])); \
		info.OneTimeMetaSafeSet<_type>(); \
		return &info; \
	} \
	virtual const DAVA::IntrospectionInfo* GetTypeInfo() const \
	{ \
		return _type::TypeInfo(); \
	}

// ����������� �������� ����� ������������. ������ � ���� �������������� ���������������.
#define MEMBER(_name, _desc, _flags) \
	new DAVA::IntrospectionMember(#_name, _desc, (int) ((long int) &((ObjectT *) 0)->_name), DAVA::MetaInfo::Instance(&ObjectT::_name), _flags),

// ����������� ����� ������������, ��� ��������. ������ � ���� �������������� ����� ������ Get/Set. 
#define PROPERTY(_name, _desc, _getter, _setter, _flags) \
	DAVA::CreateIntrospectionProperty(#_name, _desc, DAVA::MetaInfo::Instance(&ObjectT::_name), &ObjectT::_getter, &ObjectT::_setter, _flags),

// ����������� ����� ������������, ��� ���������. ������ - ��. IntrospectionCollection
#define COLLECTION(_name, _desc, _flags) \
	DAVA::CreateIntrospectionCollection(&((ObjectT *) 0)->_name, #_name, _desc, (int) ((long int) &((ObjectT *) 0)->_name), DAVA::MetaInfo::Instance(&ObjectT::_name), _flags),

#endif // __DAVAENGINE_INTROSPECTION_H__
