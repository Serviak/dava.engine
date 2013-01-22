/*
	DAVA SDK
	Meta
	Author: Sergey Zdanevich
*/

#ifndef __DAVAENGINE_META_H__
#define __DAVAENGINE_META_H__

#include "Base/IntrospectionBase.h"

namespace DAVA
{
	// ����� ����-���������� �����.
	struct MetaInfo
	{
		// MetaInfo::Instance<�>() - ���������� ���������� ��� ���� � ��������� �� ����-����������.
		template <typename MetaT>
		static MetaInfo *Instance()
		{
			// ��� ����, ����� ������������� ���������� � ������������ ��������� �� ����-���������� ��� ���� �
			// ��� �� ��������� ��� ����������� ����������.
			static MetaInfo metaInfo(typeid(MetaT).name(), sizeof(MetaT), PointerTraits<MetaT>::result);

			// ��������� ����� ������������ ��� ������� ���� � � ��������� �� � ������ ����-���������� (��).
			// ������ ���� ������������ �� ������ � ������������ ��? ������, ��� � ���� ������ ��� �� ������������� ����������� ����������� ��
			// ���������� �������� ������� �����������, ������� � ���� �������, ��� ��������, ��������� ������� ������ ��. ��� �������� � ����������
			// ��� �� �������������� ������������ ��.
            metaInfo.OneTimeIntrospectionSafeSet<MetaT>();

			// ���������� ������� ����-����������.
			return &metaInfo;
		}

		// MetaInfo::Instance<C, �>(T C::*var) - ��������� ��������� �� ���� ������ � ���� � � ���������� 
		// ���������� ��� � ��������� �� ����-����������. 
		//
		// ������ �������-������ ������������ ��� ���������� ������������.
		// �� ������������� �� ������������� ��������.
		template <typename ClassT, typename MemberT>
		static MetaInfo* Instance(MemberT ClassT::*var)
		{
			return MetaInfo::Instance<MemberT>();
		}

		// ������ ���� � ������
		inline int GetSize() const
		{
			return type_size;
		}

		// ��� ����
		// ��������� ������ ������� ������ ������� �� ��������� � ����������� � ������������� ���
		// ������������� ������������� ������ ��� �������.
		//
		// ��������� �������� �������� ��������� ����:
		// ("int" == meta->GetTypeName())
		//
		// ���������� �������:
		// (MetaInto::Instance<int>() == meta)
		inline const char* GetTypeName() const
		{
			return type_name;
		}

		// ���������� true ���� ������ ��� �������� ����������
		inline bool IsPointer() const
		{
			return isPointer;
		}
        
		// ��������� ������������ ��� ������� ����
		// � ������ ���������� ������������ �������� ������ NULL
		//
		// �����: ��������� ������ ������ ����� ��������� ��� ���� � � �*
		// ������������ ������ ������������ ������� IsPointer() ��� ����������
		inline const IntrospectionInfo* GetIntrospection() const
		{
			return introspection;
		}

	protected:
		// �������� ���������� � ������� ����-���������� T ��������� �� ������������ 
		// ��������� ���� IntrospectionT (IntrospectionT == T ������)
        template<typename IntrospectionT>
        void OneTimeIntrospectionSafeSet()
        {
            if(!introspectionOneTimeSet)
            {
                introspectionOneTimeSet = true;

				// ��� �������� ������ ������:
				// typename Select<res1, T1, T2>::Result
				// ���� ��������� res1 �������� �������, �� ���������� ��� T1 ����� T2
				//
				// �.�. <typename Select<PointerTraits<res1, T1, T2>::Result>, 
				//
				// ��� res1 == true ������������� �
				//
				// DAVA::GetIntrospection<typename T1>
				//
				// PointerTraits<T>::result - ������ true, ���� ��� � �������� ����������, �
				// PointerTraits<IntrospectionT>::PointeeType - ������ ��� ��������� 
                introspection = DAVA::GetIntrospection<typename Select<PointerTraits<IntrospectionT>::result, typename PointerTraits<IntrospectionT>::PointeeType, IntrospectionT>::Result>();
            }
        }

	private:
		MetaInfo(const char *_type_name, int _type_size, bool is_pointer)
			: type_name(_type_name)
			, type_size(_type_size)
            , introspection(NULL)
            , introspectionOneTimeSet(false)
			, isPointer(is_pointer)
		{ }

		const int type_size;
		const char *type_name;
        const IntrospectionInfo *introspection;
        
        bool introspectionOneTimeSet;
		bool isPointer;
	};
};

#endif // __DAVAENGINE_META_H__
