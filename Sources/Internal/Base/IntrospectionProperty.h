#ifndef __DAVAENGINE_INTROSPECTION_PROPERTY_H__
#define __DAVAENGINE_INTROSPECTION_PROPERTY_H__

#include "Base/IntrospectionBase.h"

namespace DAVA
{
	// Класс представляет расширение базового класса IntrospectionMember и описывает члена интроспекции, как свойство
	// Свойство в отличии от базового члена интроспекции позволяет указать методы класса, 
	// которые будут использоваться при получении/установки значений в данный член интроспекции.
	// Свойства требуются в том случае, если логика работы класса такова, что при изменение одной переменной
	// может поменять другая.
	// 
	// В классе переопределены виртуальные функции базового класса - IntrospectionMember: Value(), SetValue()
	// 
	// Пример:
	// class A
	// {
	// public:
	//		int sum;
	//		
	//		int GetA() {return a;};
	//		int GetB() {return b;};
	//		
	//		void SetA(int _a) {a = _a; sum = a + b;};
	//		void SetB(int _b) {b = _b; sum = a + b;};
	//		
	// protected:
	//		int a;
	//		int b;
	//		
	// public:
	//		INTROSPECTION(A, 
	//			MEMBER(sum, "Sum a+b", 0)
	//			PROPERTY(a, "a value", GetA, SetA, 0)
	//			PROPERTY(b, "b value", GetB, SetB, 0)
	//		);
	// };

	template<typename T, typename V>
	class IntrospectionProperty : public IntrospectionMember
	{
	public:
		typedef V	 (T::*GetterPtr)() const;
		typedef void (T::*SetterPtr)(const V &);

		IntrospectionProperty(const char *_name, const char *_desc, const MetaInfo *_type, GetterPtr _g, SetterPtr _s, int _flags)
			: IntrospectionMember(_name, _desc, 0, _type, _flags), getter(_g), setter(_s)
		{ }

		virtual VariantType Value(void *object) const
		{
			T* realObj = (T *) object;
			V realValue = (realObj->*getter)();
			return VariantType::LoadData(&realValue, DAVA::MetaInfo::Instance<V>());
		}

		virtual void SetValue(void *object, const VariantType &val) const
		{
			T* realObj = (T *) object;
			V realValue;
			VariantType::SaveData(&realValue, DAVA::MetaInfo::Instance<V>(), val);
			(realObj->*setter)(realValue);
		}

		virtual void* Pointer(void *object) const { return NULL; };
		virtual void* Data(void *object) const { return NULL; };

	protected:
		const GetterPtr getter;
		const SetterPtr setter;
	};

	// Этот класс по сути специализация IntrospectionProperty, с той лишь разницой, что
	// возвращаемое Get функцией значение является ссылкой.
	template<typename T, typename V>
	class IntrospectionPropertyReturnRef : public IntrospectionMember
	{
	public:
		typedef V&	 (T::*GetterPtr)() const;
		typedef void (T::*SetterPtr)(const V &);

		IntrospectionPropertyReturnRef(const char *_name, const char *_desc, const MetaInfo *_type, GetterPtr _g, SetterPtr _s, int _flags)
			: IntrospectionMember(_name, _desc, 0, _type, _flags), getter(_g), setter(_s)
		{ }

		virtual VariantType Value(void *object) const
		{
			T* realObj = (T *) object;
			V& realValue = (realObj->*getter)();
			return VariantType::LoadData(&realValue, DAVA::MetaInfo::Instance<V>());
		}

		virtual void SetValue(void *object, const VariantType &val) const
		{
			T* realObj = (T *) object;
			V realValue;
			VariantType::SaveData(&realValue, DAVA::MetaInfo::Instance<V>(), val);
			(realObj->*setter)(realValue);
		}

		virtual void* Pointer(void *object) const { return NULL; };
		virtual void* Data(void *object) const { return NULL; };

	protected:
		const GetterPtr getter;
		const SetterPtr setter;
	};

	// Этот класс по сути специализация IntrospectionProperty, с той лишь разницой, что
	// аргумертом Set/Get функций является указатель.
	template<typename T, typename V>
	class IntrospectionPropertyParamRef : public IntrospectionMember
	{
	public:
		typedef V*	 (T::*GetterPtr)();
		typedef void (T::*SetterPtr)(V*);

		IntrospectionPropertyParamRef(const char *_name, const char *_desc, const MetaInfo *_type, GetterPtr _g, SetterPtr _s, int _flags)
			: IntrospectionMember(_name, _desc, 0, _type, _flags), getter(_g), setter(_s)
		{ }

		virtual VariantType Value(void *object) const
		{
			T* realObj = (T *) object;
			V* realValue = (realObj->*getter)();
			return VariantType::LoadData(&realValue, DAVA::MetaInfo::Instance<V>());
		}

		virtual void SetValue(void *object, const VariantType &val) const
		{
			T* realObj = (T *) object;
			V* realValue;
			VariantType::SaveData(&realValue, DAVA::MetaInfo::Instance<V>(), val);
			(realObj->*setter)(realValue);
		}

		virtual void* Pointer(void *object) const { return NULL; };
		virtual void* Data(void *object) const { return NULL; };

	protected:
		const GetterPtr getter;
		const SetterPtr setter;
	};

	// Этот класс по сути специализация IntrospectionProperty, с той лишь разницой, что
	// аргумертом Set/Get функций является обычный тип.
	template<typename T, typename V>
	class IntrospectionPropertyParamSimple : public IntrospectionMember
	{
	public:
		typedef V	 (T::*GetterPtr)();
		typedef void (T::*SetterPtr)(V);

		IntrospectionPropertyParamSimple(const char *_name, const char *_desc, const MetaInfo *_type, GetterPtr _g, SetterPtr _s, int _flags)
			: IntrospectionMember(_name, _desc, 0, _type, _flags), getter(_g), setter(_s)
		{ }

		virtual VariantType Value(void *object) const
		{
			T* realObj = (T *) object;
			V realValue = (realObj->*getter)();
			return VariantType::LoadData(&realValue, DAVA::MetaInfo::Instance<V>());
		}

		virtual void SetValue(void *object, const VariantType &val) const
		{
			T* realObj = (T *) object;
			V realValue;
			VariantType::SaveData(&realValue, DAVA::MetaInfo::Instance<V>(), val);
			(realObj->*setter)(realValue);
		}

		virtual void* Pointer(void *object) const { return NULL; };
		virtual void* Data(void *object) const { return NULL; };

	protected:
		const GetterPtr getter;
		const SetterPtr setter;
	};

	// Набор функций для автоматического вывода параметро и создания IntrospectionProperty или IntrospectionPropertyRef
	// в зависимости от входных типов
	template<typename TT, typename VV>
	DAVA::IntrospectionMember* CreateIntrospectionProperty(const char *_name, const char *_desc, VV (TT::*_g)(), void (TT::*_s)(const VV&), int _flags)
	{
		return new IntrospectionProperty<TT,VV>(_name, _desc, DAVA::MetaInfo::Instance<typename VV>(), (VV (TT::*)() const) _g, _s, _flags);
	}

	template<typename TT, typename VV>
	DAVA::IntrospectionMember* CreateIntrospectionProperty(const char *_name, const char *_desc, VV (TT::*_g)() const, void (TT::*_s)(const VV&), int _flags)
	{
		return new IntrospectionProperty<TT,VV>(_name, _desc, DAVA::MetaInfo::Instance<typename VV>(), _g, _s, _flags);
	}

	template<typename TT, typename VV>
	DAVA::IntrospectionMember* CreateIntrospectionProperty(const char *_name, const char *_desc, const VV (TT::*_g)(), void (TT::*_s)(const VV&), int _flags)
	{
		return new IntrospectionProperty<TT,VV>(_name, _desc, DAVA::MetaInfo::Instance<typename VV>(), (VV (TT::*)() const) _g, _s, _flags);
	}

	template<typename TT, typename VV>
	DAVA::IntrospectionMember* CreateIntrospectionProperty(const char *_name, const char *_desc, const VV (TT::*_g)() const, void (TT::*_s)(const VV&), int _flags)
	{
		return new IntrospectionProperty<TT,VV>(_name, _desc, DAVA::MetaInfo::Instance<typename VV>(), (VV (TT::*)() const) _g, _s, _flags);
	}

	// ret ref
	template<typename TT, typename VV>
	DAVA::IntrospectionMember* CreateIntrospectionProperty(const char *_name, const char *_desc, VV& (TT::*_g)(), void (TT::*_s)(const VV&), int _flags)
	{
		return new IntrospectionPropertyReturnRef<TT,VV>(_name, _desc, DAVA::MetaInfo::Instance<typename VV>(), (VV& (TT::*)() const) _g, _s, _flags);
	}

	template<typename TT, typename VV>
	DAVA::IntrospectionMember* CreateIntrospectionProperty(const char *_name, const char *_desc, VV& (TT::*_g)() const, void (TT::*_s)(const VV&), int _flags)
	{
		return new IntrospectionPropertyReturnRef<TT,VV>(_name, _desc, DAVA::MetaInfo::Instance<typename VV>(), (VV& (TT::*)() const) _g, _s, _flags);
	}

	template<typename TT, typename VV>
	DAVA::IntrospectionMember* CreateIntrospectionProperty(const char *_name, const char *_desc, const VV& (TT::*_g)(), void (TT::*_s)(const VV&), int _flags)
	{
		return new IntrospectionPropertyReturnRef<TT,VV>(_name, _desc, DAVA::MetaInfo::Instance<typename VV>(), (VV& (TT::*)() const) _g, _s, _flags);
	}

	template<typename TT, typename VV>
	DAVA::IntrospectionMember* CreateIntrospectionProperty(const char *_name, const char *_desc, const VV& (TT::*_g)() const, void (TT::*_s)(const VV&), int _flags)
	{
		return new IntrospectionPropertyReturnRef<TT,VV>(_name, _desc, DAVA::MetaInfo::Instance<typename VV>(), (VV& (TT::*)() const) _g, _s, _flags);
	}


	// param pointer
	template<typename TT, typename VV>
	DAVA::IntrospectionMember* CreateIntrospectionProperty(const char *_name, const char *_desc, VV* (TT::*_g)(), void (TT::*_s)(const VV*), int _flags)
	{
		return new IntrospectionPropertyParamRef<TT,VV>(_name, _desc, DAVA::MetaInfo::Instance<typename VV*>(), (VV* (TT::*)()) _g, (void (TT::*)(VV *)) _s, _flags);
	}

	template<typename TT, typename VV>
	DAVA::IntrospectionMember* CreateIntrospectionProperty(const char *_name, const char *_desc, VV* (TT::*_g)() const, void (TT::*_s)(const VV*), int _flags)
	{
		return new IntrospectionPropertyParamRef<TT,VV>(_name, _desc, DAVA::MetaInfo::Instance<typename VV*>(), (VV* (TT::*)()) _g, (void (TT::*)(VV *)) _s, _flags);
	}

	template<typename TT, typename VV>
	DAVA::IntrospectionMember* CreateIntrospectionProperty(const char *_name, const char *_desc, const VV* (TT::*_g)(), void (TT::*_s)(const VV*), int _flags)
	{
		return new IntrospectionPropertyParamRef<TT,VV>(_name, _desc, DAVA::MetaInfo::Instance<typename VV*>(), (VV* (TT::*)()) _g, (void (TT::*)(VV *)) _s, _flags);
	}

	template<typename TT, typename VV>
	DAVA::IntrospectionMember* CreateIntrospectionProperty(const char *_name, const char *_desc, const VV* (TT::*_g)() const, void (TT::*_s)(const VV*), int _flags)
	{
		return new IntrospectionPropertyParamRef<TT,VV>(_name, _desc, DAVA::MetaInfo::Instance<typename VV*>(), (VV* (TT::*)()) _g, (void (TT::*)(VV *)) _s, _flags);
	}

	template<typename TT, typename VV>
	DAVA::IntrospectionMember* CreateIntrospectionProperty(const char *_name, const char *_desc, VV* (TT::*_g)(), void (TT::*_s)(VV*), int _flags)
	{
		return new IntrospectionPropertyParamRef<TT,VV>(_name, _desc, DAVA::MetaInfo::Instance<typename VV*>(), (VV* (TT::*)()) _g, (void (TT::*)(VV *)) _s, _flags);
	}

	template<typename TT, typename VV>
	DAVA::IntrospectionMember* CreateIntrospectionProperty(const char *_name, const char *_desc, VV* (TT::*_g)() const, void (TT::*_s)(VV*), int _flags)
	{
		return new IntrospectionPropertyParamRef<TT,VV>(_name, _desc, DAVA::MetaInfo::Instance<typename VV*>(), (VV* (TT::*)()) _g, (void (TT::*)(VV *)) _s, _flags);
	}

	template<typename TT, typename VV>
	DAVA::IntrospectionMember* CreateIntrospectionProperty(const char *_name, const char *_desc, const VV* (TT::*_g)(), void (TT::*_s)(VV*), int _flags)
	{
		return new IntrospectionPropertyParamRef<TT,VV>(_name, _desc, DAVA::MetaInfo::Instance<typename VV*>(), (VV* (TT::*)()) _g, (void (TT::*)(VV *)) _s, _flags);
	}

	template<typename TT, typename VV>
	DAVA::IntrospectionMember* CreateIntrospectionProperty(const char *_name, const char *_desc, const VV* (TT::*_g)() const, void (TT::*_s)(VV*), int _flags)
	{
		return new IntrospectionPropertyParamRef<TT,VV>(_name, _desc, DAVA::MetaInfo::Instance<typename VV*>(), (VV* (TT::*)()) _g, (void (TT::*)(VV *)) _s, _flags);
	}

	// param simple
	template<typename TT>
	DAVA::IntrospectionMember* CreateIntrospectionProperty(const char *_name, const char *_desc, DAVA::int32 (TT::*_g)(), void (TT::*_s)(DAVA::int32), int _flags)
	{
		return new IntrospectionPropertyParamSimple<TT, DAVA::int32>(_name, _desc, DAVA::MetaInfo::Instance<DAVA::int32>(), _g, _s, _flags);
	}

	template<typename TT>
	DAVA::IntrospectionMember* CreateIntrospectionProperty(const char *_name, const char *_desc, DAVA::uint32 (TT::*_g)(), void (TT::*_s)(DAVA::uint32), int _flags)
	{
		return new IntrospectionPropertyParamSimple<TT, DAVA::uint32>(_name, _desc, DAVA::MetaInfo::Instance<DAVA::uint32>(), _g, _s, _flags);
	}

	template<typename TT>
	DAVA::IntrospectionMember* CreateIntrospectionProperty(const char *_name, const char *_desc, DAVA::int64 (TT::*_g)(), void (TT::*_s)(DAVA::int64), int _flags)
	{
		return new IntrospectionPropertyParamSimple<TT, DAVA::int64>(_name, _desc, DAVA::MetaInfo::Instance<DAVA::int64>(), _g, _s, _flags);
	}

	template<typename TT>
	DAVA::IntrospectionMember* CreateIntrospectionProperty(const char *_name, const char *_desc, DAVA::uint64 (TT::*_g)(), void (TT::*_s)(DAVA::uint64), int _flags)
	{
		return new IntrospectionPropertyParamSimple<TT, DAVA::uint64>(_name, _desc, DAVA::MetaInfo::Instance<DAVA::uint64>(), _g, _s, _flags);
	}

	template<typename TT>
	DAVA::IntrospectionMember* CreateIntrospectionProperty(const char *_name, const char *_desc, bool (TT::*_g)(), void (TT::*_s)(bool), int _flags)
	{
		return new IntrospectionPropertyParamSimple<TT, bool>(_name, _desc, DAVA::MetaInfo::Instance<bool>(), _g, _s, _flags);
	}

	template<typename TT>
	DAVA::IntrospectionMember* CreateIntrospectionProperty(const char *_name, const char *_desc, DAVA::int8 (TT::*_g)(), void (TT::*_s)(DAVA::int8), int _flags)
	{
		return new IntrospectionPropertyParamSimple<TT, DAVA::int8>(_name, _desc, DAVA::MetaInfo::Instance<DAVA::int8>(), _g, _s, _flags);
	}

	template<typename TT>
	DAVA::IntrospectionMember* CreateIntrospectionProperty(const char *_name, const char *_desc, DAVA::uint8 (TT::*_g)(), void (TT::*_s)(DAVA::uint8), int _flags)
	{
		return new IntrospectionPropertyParamSimple<TT, DAVA::uint8>(_name, _desc, DAVA::MetaInfo::Instance<DAVA::uint8>(), _g, _s, _flags);
	}

	template<typename TT>
	DAVA::IntrospectionMember* CreateIntrospectionProperty(const char *_name, const char *_desc, DAVA::int16 (TT::*_g)(), void (TT::*_s)(DAVA::int16), int _flags)
	{
		return new IntrospectionPropertyParamSimple<TT, DAVA::int16>(_name, _desc, DAVA::MetaInfo::Instance<DAVA::int16>(), _g, _s, _flags);
	}

	template<typename TT>
	DAVA::IntrospectionMember* CreateIntrospectionProperty(const char *_name, const char *_desc, DAVA::uint16 (TT::*_g)(), void (TT::*_s)(DAVA::uint16), int _flags)
	{
		return new IntrospectionPropertyParamSimple<TT, DAVA::uint16>(_name, _desc, DAVA::MetaInfo::Instance<DAVA::uint16>(), _g, _s, _flags);
	}

	template<typename TT>
	DAVA::IntrospectionMember* CreateIntrospectionProperty(const char *_name, const char *_desc, DAVA::char8 (TT::*_g)(), void (TT::*_s)(DAVA::char8), int _flags)
	{
		return new IntrospectionPropertyParamSimple<TT, DAVA::char8>(_name, _desc, DAVA::MetaInfo::Instance<DAVA::char8>(), _g, _s, _flags);
	}

	template<typename TT>
	DAVA::IntrospectionMember* CreateIntrospectionProperty(const char *_name, const char *_desc, DAVA::char16 (TT::*_g)(), void (TT::*_s)(DAVA::char16), int _flags)
	{
		return new IntrospectionPropertyParamSimple<TT, DAVA::char16>(_name, _desc, DAVA::MetaInfo::Instance<DAVA::char16>(), _g, _s, _flags);
	}

	template<typename TT>
	DAVA::IntrospectionMember* CreateIntrospectionProperty(const char *_name, const char *_desc, DAVA::float32 (TT::*_g)(), void (TT::*_s)(DAVA::float32), int _flags)
	{
		return new IntrospectionPropertyParamSimple<TT, DAVA::float32>(_name, _desc, DAVA::MetaInfo::Instance<DAVA::float32>(), _g, _s, _flags);
	}

	template<typename TT>
	DAVA::IntrospectionMember* CreateIntrospectionProperty(const char *_name, const char *_desc, DAVA::float64 (TT::*_g)(), void (TT::*_s)(DAVA::float64), int _flags)
	{
		return new IntrospectionPropertyParamSimple<TT, DAVA::float64>(_name, _desc, DAVA::MetaInfo::Instance<DAVA::float64>(), _g, _s, _flags);
	}
};

#endif // __DAVAENGINE_INTROSPECTION_PROPERTY_H__
