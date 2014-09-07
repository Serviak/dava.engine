#ifndef __DAVA_BIND_H__
#define __DAVA_BIND_H__

#include "TypeTraits.h"
#include "TypeList.h"
#include "Function.h"
#include "FunctionTraits.h"
#include "TypeHolders.h"

namespace DAVA
{
	// ====================================================================================================================================================
	// Bound helper class
	// ====================================================================================================================================================
	template<typename IncomingFunctionType, typename P1 = NullType, typename P2 = NullType, typename P3 = NullType, typename P4 = NullType, typename P5 = NullType, typename P6 = NullType, typename P7 = NullType, typename P8 = NullType>
	struct BoundHelper
	{
		// It is allowed only DAVA::Function to be IncomingFunctionType
		typedef typename IncomingFunctionType::IsFunctionType OnlyDAVAFunctionIsAllowedToBeBinded;

		typedef typename IncomingFunctionType::ReturnType ReturnType;
		typedef typename IncomingFunctionType::ParamType1 ParamType1;
		typedef typename IncomingFunctionType::ParamType2 ParamType2;
		typedef typename IncomingFunctionType::ParamType3 ParamType3;
		typedef typename IncomingFunctionType::ParamType4 ParamType4;
		typedef typename IncomingFunctionType::ParamType5 ParamType5;
		typedef typename IncomingFunctionType::ParamType6 ParamType6;
		typedef typename IncomingFunctionType::ParamType7 ParamType7;
		typedef typename IncomingFunctionType::ParamType8 ParamType8;

		typedef typename Select<IsPlaceholder<P1>::result, ParamType1, NullType>::Result RealParamType1;
		typedef typename Select<IsPlaceholder<P2>::result, ParamType2, NullType>::Result RealParamType2;
		typedef typename Select<IsPlaceholder<P3>::result, ParamType3, NullType>::Result RealParamType3;
		typedef typename Select<IsPlaceholder<P4>::result, ParamType4, NullType>::Result RealParamType4;
		typedef typename Select<IsPlaceholder<P5>::result, ParamType5, NullType>::Result RealParamType5;
		typedef typename Select<IsPlaceholder<P6>::result, ParamType6, NullType>::Result RealParamType6;
		typedef typename Select<IsPlaceholder<P7>::result, ParamType7, NullType>::Result RealParamType7;
		typedef typename Select<IsPlaceholder<P8>::result, ParamType8, NullType>::Result RealParamType8;

		typedef Typelist<NullType, NullType> RealParamsList0;
		typedef typename TL::Append<RealParamsList0, RealParamType1>::Result RealParamsList1;
		typedef typename TL::Append<RealParamsList1, RealParamType2>::Result RealParamsList2;
		typedef typename TL::Append<RealParamsList2, RealParamType3>::Result RealParamsList3;
		typedef typename TL::Append<RealParamsList3, RealParamType4>::Result RealParamsList4;
		typedef typename TL::Append<RealParamsList4, RealParamType5>::Result RealParamsList5;
		typedef typename TL::Append<RealParamsList5, RealParamType6>::Result RealParamsList6;
		typedef typename TL::Append<RealParamsList6, RealParamType7>::Result RealParamsList7;
		typedef typename TL::Append<RealParamsList7, RealParamType8>::Result RealParamsList8;
		typedef typename RealParamsList8 RealParamsFullList;

		// possible outgoing function types
		typedef typename Function<ReturnType (
			)> OutgoingFunctionType0;

		typedef typename Function<ReturnType(
			typename TL::TypeAtNonStrict<RealParamsFullList, SelectIndex<IsPlaceholder<P1>::result, PlaceholderIndex<P1>::value, 1>::result >::Result
			)> OutgoingFunctionType1;

		typedef typename Function<ReturnType(
			typename TL::TypeAtNonStrict<RealParamsFullList, SelectIndex<IsPlaceholder<P1>::result, PlaceholderIndex<P1>::value, 1>::result >::Result,
			typename TL::TypeAtNonStrict<RealParamsFullList, SelectIndex<IsPlaceholder<P2>::result, PlaceholderIndex<P2>::value, 2>::result >::Result
			)> OutgoingFunctionType2;

		typedef typename Function<ReturnType(
			typename TL::TypeAtNonStrict<RealParamsFullList, SelectIndex<IsPlaceholder<P1>::result, PlaceholderIndex<P1>::value, 1>::result >::Result,
			typename TL::TypeAtNonStrict<RealParamsFullList, SelectIndex<IsPlaceholder<P2>::result, PlaceholderIndex<P2>::value, 2>::result >::Result,
			typename TL::TypeAtNonStrict<RealParamsFullList, SelectIndex<IsPlaceholder<P3>::result, PlaceholderIndex<P3>::value, 3>::result >::Result
			)> OutgoingFunctionType3;

		typedef typename Function<ReturnType(
			typename TL::TypeAtNonStrict<RealParamsFullList, SelectIndex<IsPlaceholder<P1>::result, PlaceholderIndex<P1>::value, 1>::result >::Result,
			typename TL::TypeAtNonStrict<RealParamsFullList, SelectIndex<IsPlaceholder<P2>::result, PlaceholderIndex<P2>::value, 2>::result >::Result,
			typename TL::TypeAtNonStrict<RealParamsFullList, SelectIndex<IsPlaceholder<P3>::result, PlaceholderIndex<P3>::value, 3>::result >::Result,
			typename TL::TypeAtNonStrict<RealParamsFullList, SelectIndex<IsPlaceholder<P4>::result, PlaceholderIndex<P4>::value, 4>::result >::Result
			)> OutgoingFunctionType4;

		typedef typename Function<ReturnType(
			typename TL::TypeAtNonStrict<RealParamsFullList, SelectIndex<IsPlaceholder<P1>::result, PlaceholderIndex<P1>::value, 1>::result >::Result,
			typename TL::TypeAtNonStrict<RealParamsFullList, SelectIndex<IsPlaceholder<P2>::result, PlaceholderIndex<P2>::value, 2>::result >::Result,
			typename TL::TypeAtNonStrict<RealParamsFullList, SelectIndex<IsPlaceholder<P3>::result, PlaceholderIndex<P3>::value, 3>::result >::Result,
			typename TL::TypeAtNonStrict<RealParamsFullList, SelectIndex<IsPlaceholder<P4>::result, PlaceholderIndex<P4>::value, 4>::result >::Result,
			typename TL::TypeAtNonStrict<RealParamsFullList, SelectIndex<IsPlaceholder<P5>::result, PlaceholderIndex<P5>::value, 5>::result >::Result
			)> OutgoingFunctionType5;

		typedef typename Function<ReturnType(
			typename TL::TypeAtNonStrict<RealParamsFullList, SelectIndex<IsPlaceholder<P1>::result, PlaceholderIndex<P1>::value, 1>::result >::Result,
			typename TL::TypeAtNonStrict<RealParamsFullList, SelectIndex<IsPlaceholder<P2>::result, PlaceholderIndex<P2>::value, 2>::result >::Result,
			typename TL::TypeAtNonStrict<RealParamsFullList, SelectIndex<IsPlaceholder<P3>::result, PlaceholderIndex<P3>::value, 3>::result >::Result,
			typename TL::TypeAtNonStrict<RealParamsFullList, SelectIndex<IsPlaceholder<P4>::result, PlaceholderIndex<P4>::value, 4>::result >::Result,
			typename TL::TypeAtNonStrict<RealParamsFullList, SelectIndex<IsPlaceholder<P5>::result, PlaceholderIndex<P5>::value, 5>::result >::Result,
			typename TL::TypeAtNonStrict<RealParamsFullList, SelectIndex<IsPlaceholder<P6>::result, PlaceholderIndex<P6>::value, 6>::result >::Result
			)> OutgoingFunctionType6;

		typedef typename Function<ReturnType(
			typename TL::TypeAtNonStrict<RealParamsFullList, SelectIndex<IsPlaceholder<P1>::result, PlaceholderIndex<P1>::value, 1>::result >::Result,
			typename TL::TypeAtNonStrict<RealParamsFullList, SelectIndex<IsPlaceholder<P2>::result, PlaceholderIndex<P2>::value, 2>::result >::Result,
			typename TL::TypeAtNonStrict<RealParamsFullList, SelectIndex<IsPlaceholder<P3>::result, PlaceholderIndex<P3>::value, 3>::result >::Result,
			typename TL::TypeAtNonStrict<RealParamsFullList, SelectIndex<IsPlaceholder<P4>::result, PlaceholderIndex<P4>::value, 4>::result >::Result,
			typename TL::TypeAtNonStrict<RealParamsFullList, SelectIndex<IsPlaceholder<P5>::result, PlaceholderIndex<P5>::value, 5>::result >::Result,
			typename TL::TypeAtNonStrict<RealParamsFullList, SelectIndex<IsPlaceholder<P6>::result, PlaceholderIndex<P6>::value, 6>::result >::Result,
			typename TL::TypeAtNonStrict<RealParamsFullList, SelectIndex<IsPlaceholder<P7>::result, PlaceholderIndex<P7>::value, 7>::result >::Result
			)> OutgoingFunctionType7;

		typedef typename Function<ReturnType(
			typename TL::TypeAtNonStrict<RealParamsFullList, SelectIndex<IsPlaceholder<P1>::result, PlaceholderIndex<P1>::value, 1>::result >::Result,
			typename TL::TypeAtNonStrict<RealParamsFullList, SelectIndex<IsPlaceholder<P2>::result, PlaceholderIndex<P2>::value, 2>::result >::Result,
			typename TL::TypeAtNonStrict<RealParamsFullList, SelectIndex<IsPlaceholder<P3>::result, PlaceholderIndex<P3>::value, 3>::result >::Result,
			typename TL::TypeAtNonStrict<RealParamsFullList, SelectIndex<IsPlaceholder<P4>::result, PlaceholderIndex<P4>::value, 4>::result >::Result,
			typename TL::TypeAtNonStrict<RealParamsFullList, SelectIndex<IsPlaceholder<P5>::result, PlaceholderIndex<P5>::value, 5>::result >::Result,
			typename TL::TypeAtNonStrict<RealParamsFullList, SelectIndex<IsPlaceholder<P6>::result, PlaceholderIndex<P6>::value, 6>::result >::Result,
			typename TL::TypeAtNonStrict<RealParamsFullList, SelectIndex<IsPlaceholder<P7>::result, PlaceholderIndex<P7>::value, 7>::result >::Result,
			typename TL::TypeAtNonStrict<RealParamsFullList, SelectIndex<IsPlaceholder<P8>::result, PlaceholderIndex<P8>::value, 8>::result >::Result
			)> OutgoingFunctionType8;

		// evaluating outgoing function type
		typedef OutgoingFunctionType0 _OutgoingFunctionType0;
		typedef typename Select<IsNullType<typename TL::TypeAtNonStrict<RealParamsFullList, 1>::Result>::result, _OutgoingFunctionType0, OutgoingFunctionType1>::Result _OutgoingFunctionType1;
		typedef typename Select<IsNullType<typename TL::TypeAtNonStrict<RealParamsFullList, 2>::Result>::result, _OutgoingFunctionType1, OutgoingFunctionType2>::Result _OutgoingFunctionType2;
		typedef typename Select<IsNullType<typename TL::TypeAtNonStrict<RealParamsFullList, 3>::Result>::result, _OutgoingFunctionType2, OutgoingFunctionType3>::Result _OutgoingFunctionType3;
		typedef typename Select<IsNullType<typename TL::TypeAtNonStrict<RealParamsFullList, 4>::Result>::result, _OutgoingFunctionType3, OutgoingFunctionType4>::Result _OutgoingFunctionType4;
		typedef typename Select<IsNullType<typename TL::TypeAtNonStrict<RealParamsFullList, 5>::Result>::result, _OutgoingFunctionType4, OutgoingFunctionType5>::Result _OutgoingFunctionType5;
		typedef typename Select<IsNullType<typename TL::TypeAtNonStrict<RealParamsFullList, 6>::Result>::result, _OutgoingFunctionType5, OutgoingFunctionType6>::Result _OutgoingFunctionType6;
		typedef typename Select<IsNullType<typename TL::TypeAtNonStrict<RealParamsFullList, 7>::Result>::result, _OutgoingFunctionType6, OutgoingFunctionType7>::Result _OutgoingFunctionType7;
		typedef typename Select<IsNullType<typename TL::TypeAtNonStrict<RealParamsFullList, 8>::Result>::result, _OutgoingFunctionType7, OutgoingFunctionType8>::Result _OutgoingFunctionType8;
		typedef _OutgoingFunctionType8 OutgoingFunctionType; // evaluated result
	};

	// ====================================================================================================================================================
	// Bound_N classes implementation 
	// TODO: description
	// ====================================================================================================================================================
	template<typename IncomingFunctionType, typename P1>
 	class Bound_1 : public RefCounter
 	{
 	public:
		typedef typename BoundHelper<IncomingFunctionType, P1> Helper;
		typedef typename Helper::ReturnType ReturnType;
		typedef typename Helper::OutgoingFunctionType OutgoingFunctionType;

		Bound_1(const IncomingFunctionType &_fn, const P1 &_p1) 
			: fn(_fn), p1(_p1)
 		{ }
 
		ReturnType invoke() { return fn(p1.GetParameter()); }

		template<typename U1>
		ReturnType invoke(U1 u1) { return fn(p1.GetParameter(u1)); }

 	protected:
		IncomingFunctionType fn;
 		ParamHolder<P1> p1;
 	};
 
	template<typename IncomingFunctionType, typename P1, typename P2>
	class Bound_2 : public RefCounter
	{
	public:
		typedef typename BoundHelper<IncomingFunctionType, P1, P2> Helper;
		typedef typename Helper::ReturnType ReturnType;
		typedef typename Helper::OutgoingFunctionType OutgoingFunctionType;

		Bound_2(const IncomingFunctionType &_fn, const P1 &_p1, const P2 &_p2)
			: fn(_fn), p1(_p1), p2(_p2)
		{ }

		ReturnType invoke() { return fn(p1.GetParameter(),
			p2.GetParameter()); }

		template<typename U1>
		ReturnType invoke(U1 u1) { return fn(p1.GetParameter(u1),
			p2.GetParameter(u1)); }

		template<typename U1, typename U2>
		ReturnType invoke(U1 u1, U2 u2) { return fn(p1.GetParameter(u1, u2),
			p2.GetParameter(u1, u2)); }

	protected:
		IncomingFunctionType fn;
		ParamHolder<P1> p1;
		ParamHolder<P2> p2;
	};

	template<typename IncomingFunctionType, typename P1, typename P2, typename P3>
	class Bound_3 : public RefCounter
	{
	public:
		typedef typename BoundHelper<IncomingFunctionType, P1, P2, P3> Helper;
		typedef typename Helper::ReturnType ReturnType;
		typedef typename Helper::OutgoingFunctionType OutgoingFunctionType;

		Bound_3(const IncomingFunctionType &_fn, const P1 &_p1, const P2 &_p2, const P3 &_p3)
			: fn(_fn), p1(_p1), p2(_p2), p3(_p3)
		{ }

		ReturnType invoke() { return fn(p1.GetParameter(),
			p2.GetParameter(),
			p3.GetParameter()); }

		template<typename U1>
		ReturnType invoke(U1 u1) { return fn(p1.GetParameter(u1),
			p2.GetParameter(u1),
			p3.GetParameter(u1)); }

		template<typename U1, typename U2>
		ReturnType invoke(U1 u1, U2 u2) { return fn(p1.GetParameter(u1, u2),
			p2.GetParameter(u1, u2),
			p3.GetParameter(u1, u2)); }

		template<typename U1, typename U2, typename U3>
		ReturnType invoke(U1 u1, U2 u2, U3 u3) { return fn(p1.GetParameter(u1, u2, u3),
			p2.GetParameter(u1, u2, u3),
			p3.GetParameter(u1, u2, u3)); }

	protected:
		IncomingFunctionType fn;
		ParamHolder<P1> p1;
		ParamHolder<P2> p2;
		ParamHolder<P3> p3;
	};

	template<typename IncomingFunctionType, typename P1, typename P2, typename P3, typename P4>
	class Bound_4 : public RefCounter
	{
	public:
		typedef typename BoundHelper<IncomingFunctionType, P1, P2, P3, P4> Helper;
		typedef typename Helper::ReturnType ReturnType;
		typedef typename Helper::OutgoingFunctionType OutgoingFunctionType;

		Bound_4(const IncomingFunctionType &_fn, const P1 &_p1, const P2 &_p2, const P3 &_p3, const P4 &_p4)
			: fn(_fn), p1(_p1), p2(_p2), p3(_p3), p4(_p4)
		{ }

		ReturnType invoke() { return fn(p1.GetParameter(),
			p2.GetParameter(),
			p3.GetParameter(),
			p4.GetParameter()); }

		template<typename U1>
		ReturnType invoke(U1 u1) { return fn(p1.GetParameter(u1),
			p2.GetParameter(u1),
			p3.GetParameter(u1),
			p4.GetParameter(u1)); }

		template<typename U1, typename U2>
		ReturnType invoke(U1 u1, U2 u2) { return fn(p1.GetParameter(u1, u2),
			p2.GetParameter(u1, u2),
			p3.GetParameter(u1, u2),
			p4.GetParameter(u1, u2)); }

		template<typename U1, typename U2, typename U3>
		ReturnType invoke(U1 u1, U2 u2, U3 u3) { return fn(p1.GetParameter(u1, u2, u3),
			p2.GetParameter(u1, u2, u3),
			p3.GetParameter(u1, u2, u3),
			p4.GetParameter(u1, u2, u3)); }

		template<typename U1, typename U2, typename U3, typename U4>
		ReturnType invoke(U1 u1, U2 u2, U3 u3, U4 u4) { return fn(p1.GetParameter(u1, u2, u3, u4),
			p2.GetParameter(u1, u2, u3, u4),
			p3.GetParameter(u1, u2, u3, u4),
			p4.GetParameter(u1, u2, u3, u4)); }

	protected:
		IncomingFunctionType fn;
		ParamHolder<P1> p1;
		ParamHolder<P2> p2;
		ParamHolder<P3> p3;
		ParamHolder<P4> p4;
	};

	template<typename IncomingFunctionType, typename P1, typename P2, typename P3, typename P4, typename P5>
	class Bound_5 : public RefCounter
	{
	public:
		typedef typename BoundHelper<IncomingFunctionType, P1, P2, P3, P4, P5> Helper;
		typedef typename Helper::ReturnType ReturnType;
		typedef typename Helper::OutgoingFunctionType OutgoingFunctionType;

		Bound_5(const IncomingFunctionType &_fn, const P1 &_p1, const P2 &_p2, const P3 &_p3, const P4 &_p4, const P5 &_p5)
			: fn(_fn), p1(_p1), p2(_p2), p3(_p3), p4(_p4), p5(_p5)
		{ }

		ReturnType invoke() { return fn(p1.GetParameter(),
			p2.GetParameter(),
			p3.GetParameter(),
			p4.GetParameter(),
			p5.GetParameter()); }

		template<typename U1>
		ReturnType invoke(U1 u1) { return fn(p1.GetParameter(u1),
			p2.GetParameter(u1),
			p3.GetParameter(u1),
			p4.GetParameter(u1),
			p5.GetParameter(u1)); }

		template<typename U1, typename U2>
		ReturnType invoke(U1 u1, U2 u2) { return fn(p1.GetParameter(u1, u2),
			p2.GetParameter(u1, u2),
			p3.GetParameter(u1, u2),
			p4.GetParameter(u1, u2),
			p5.GetParameter(u1, u2)); }

		template<typename U1, typename U2, typename U3>
		ReturnType invoke(U1 u1, U2 u2, U3 u3) { return fn(p1.GetParameter(u1, u2, u3),
			p2.GetParameter(u1, u2, u3),
			p3.GetParameter(u1, u2, u3),
			p4.GetParameter(u1, u2, u3),
			p5.GetParameter(u1, u2, u3)); }

		template<typename U1, typename U2, typename U3, typename U4>
		ReturnType invoke(U1 u1, U2 u2, U3 u3, U4 u4) { return fn(p1.GetParameter(u1, u2, u3, u4),
			p2.GetParameter(u1, u2, u3, u4),
			p3.GetParameter(u1, u2, u3, u4),
			p4.GetParameter(u1, u2, u3, u4),
			p5.GetParameter(u1, u2, u3, u4)); }

		template<typename U1, typename U2, typename U3, typename U4, typename U5>
		ReturnType invoke(U1 u1, U2 u2, U3 u3, U4 u4, U5 u5) { return fn(p1.GetParameter(u1, u2, u3, u4, u5),
			p2.GetParameter(u1, u2, u3, u4, u5),
			p3.GetParameter(u1, u2, u3, u4, u5),
			p4.GetParameter(u1, u2, u3, u4, u5),
			p5.GetParameter(u1, u2, u3, u4, u5)); }

	protected:
		IncomingFunctionType fn;
		ParamHolder<P1> p1;
		ParamHolder<P2> p2;
		ParamHolder<P3> p3;
		ParamHolder<P4> p4;
		ParamHolder<P5> p5;
	};

	template<typename IncomingFunctionType, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6>
	class Bound_6 : public RefCounter
	{
	public:
		typedef typename BoundHelper<IncomingFunctionType, P1, P2, P3, P4, P5, P6> Helper;
		typedef typename Helper::ReturnType ReturnType;
		typedef typename Helper::OutgoingFunctionType OutgoingFunctionType;

		Bound_6(const IncomingFunctionType &_fn, const P1 &_p1, const P2 &_p2, const P3 &_p3, const P4 &_p4, const P5 &_p5, const P6 &_p6)
			: fn(_fn), p1(_p1), p2(_p2), p3(_p3), p4(_p4), p5(_p5), p6(_p6)
		{ }

		ReturnType invoke() { return fn(p1.GetParameter(),
			p2.GetParameter(),
			p3.GetParameter(),
			p4.GetParameter(),
			p5.GetParameter(),
			p6.GetParameter()); }

		template<typename U1>
		ReturnType invoke(U1 u1) { return fn(p1.GetParameter(u1),
			p2.GetParameter(u1),
			p3.GetParameter(u1),
			p4.GetParameter(u1),
			p5.GetParameter(u1),
			p6.GetParameter(u1)); }

		template<typename U1, typename U2>
		ReturnType invoke(U1 u1, U2 u2) { return fn(p1.GetParameter(u1, u2),
			p2.GetParameter(u1, u2),
			p3.GetParameter(u1, u2),
			p4.GetParameter(u1, u2),
			p5.GetParameter(u1, u2),
			p6.GetParameter(u1, u2)); }

		template<typename U1, typename U2, typename U3>
		ReturnType invoke(U1 u1, U2 u2, U3 u3) { return fn(p1.GetParameter(u1, u2, u3),
			p2.GetParameter(u1, u2, u3),
			p3.GetParameter(u1, u2, u3),
			p4.GetParameter(u1, u2, u3),
			p5.GetParameter(u1, u2, u3),
			p6.GetParameter(u1, u2, u3)); }

		template<typename U1, typename U2, typename U3, typename U4>
		ReturnType invoke(U1 u1, U2 u2, U3 u3, U4 u4) { return fn(p1.GetParameter(u1, u2, u3, u4),
			p2.GetParameter(u1, u2, u3, u4),
			p3.GetParameter(u1, u2, u3, u4),
			p4.GetParameter(u1, u2, u3, u4),
			p5.GetParameter(u1, u2, u3, u4),
			p6.GetParameter(u1, u2, u3, u4)); }

		template<typename U1, typename U2, typename U3, typename U4, typename U5>
		ReturnType invoke(U1 u1, U2 u2, U3 u3, U4 u4, U5 u5) { return fn(p1.GetParameter(u1, u2, u3, u4, u5),
			p2.GetParameter(u1, u2, u3, u4, u5),
			p3.GetParameter(u1, u2, u3, u4, u5),
			p4.GetParameter(u1, u2, u3, u4, u5),
			p5.GetParameter(u1, u2, u3, u4, u5),
			p6.GetParameter(u1, u2, u3, u4, u5)); }

		template<typename U1, typename U2, typename U3, typename U4, typename U5, typename U6>
		ReturnType invoke(U1 u1, U2 u2, U3 u3, U4 u4, U5 u5, U6 u6) { return fn(p1.GetParameter(u1, u2, u3, u4, u5, u6),
			p2.GetParameter(u1, u2, u3, u4, u5, u6),
			p3.GetParameter(u1, u2, u3, u4, u5, u6),
			p4.GetParameter(u1, u2, u3, u4, u5, u6),
			p5.GetParameter(u1, u2, u3, u4, u5, u6),
			p6.GetParameter(u1, u2, u3, u4, u5, u6)); }

	protected:
		IncomingFunctionType fn;
		ParamHolder<P1> p1;
		ParamHolder<P2> p2;
		ParamHolder<P3> p3;
		ParamHolder<P4> p4;
		ParamHolder<P5> p5;
		ParamHolder<P6> p6;
	};

	template<typename IncomingFunctionType, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7>
	class Bound_7 : public RefCounter
	{
	public:
		typedef typename BoundHelper<IncomingFunctionType, P1, P2, P3, P4, P5, P6, P7> Helper;
		typedef typename Helper::ReturnType ReturnType;
		typedef typename Helper::OutgoingFunctionType OutgoingFunctionType;

		Bound_7(const IncomingFunctionType &_fn, const P1 &_p1, const P2 &_p2, const P3 &_p3, const P4 &_p4, const P5 &_p5, const P6 &_p6, const P7 &_p7)
			: fn(_fn), p1(_p1), p2(_p2), p3(_p3), p4(_p4), p5(_p5), p6(_p6), p7(_p7)
		{ }

		ReturnType invoke() {
			return fn(p1.GetParameter(),
				p2.GetParameter(),
				p3.GetParameter(),
				p4.GetParameter(),
				p5.GetParameter(),
				p6.GetParameter(),
				p7.GetParameter());
		}

		template<typename U1>
		ReturnType invoke(U1 u1) {
			return fn(p1.GetParameter(u1),
				p2.GetParameter(u1),
				p3.GetParameter(u1),
				p4.GetParameter(u1),
				p5.GetParameter(u1),
				p6.GetParameter(u1),
				p7.GetParameter(u1));
		}

		template<typename U1, typename U2>
		ReturnType invoke(U1 u1, U2 u2) {
			return fn(p1.GetParameter(u1, u2),
				p2.GetParameter(u1, u2),
				p3.GetParameter(u1, u2),
				p4.GetParameter(u1, u2),
				p5.GetParameter(u1, u2),
				p6.GetParameter(u1, u2),
				p7.GetParameter(u1, u2));
		}

		template<typename U1, typename U2, typename U3>
		ReturnType invoke(U1 u1, U2 u2, U3 u3) {
			return fn(p1.GetParameter(u1, u2, u3),
				p2.GetParameter(u1, u2, u3),
				p3.GetParameter(u1, u2, u3),
				p4.GetParameter(u1, u2, u3),
				p5.GetParameter(u1, u2, u3),
				p6.GetParameter(u1, u2, u3),
				p7.GetParameter(u1, u2, u3));
		}

		template<typename U1, typename U2, typename U3, typename U4>
		ReturnType invoke(U1 u1, U2 u2, U3 u3, U4 u4) {
			return fn(p1.GetParameter(u1, u2, u3, u4),
				p2.GetParameter(u1, u2, u3, u4),
				p3.GetParameter(u1, u2, u3, u4),
				p4.GetParameter(u1, u2, u3, u4),
				p5.GetParameter(u1, u2, u3, u4),
				p6.GetParameter(u1, u2, u3, u4),
				p7.GetParameter(u1, u2, u3, u4));
		}

		template<typename U1, typename U2, typename U3, typename U4, typename U5>
		ReturnType invoke(U1 u1, U2 u2, U3 u3, U4 u4, U5 u5) {
			return fn(p1.GetParameter(u1, u2, u3, u4, u5),
				p2.GetParameter(u1, u2, u3, u4, u5),
				p3.GetParameter(u1, u2, u3, u4, u5),
				p4.GetParameter(u1, u2, u3, u4, u5),
				p5.GetParameter(u1, u2, u3, u4, u5),
				p6.GetParameter(u1, u2, u3, u4, u5),
				p7.GetParameter(u1, u2, u3, u4, u5));
		}

		template<typename U1, typename U2, typename U3, typename U4, typename U5, typename U6>
		ReturnType invoke(U1 u1, U2 u2, U3 u3, U4 u4, U5 u5, U6 u6) {
			return fn(p1.GetParameter(u1, u2, u3, u4, u5, u6),
				p2.GetParameter(u1, u2, u3, u4, u5, u6),
				p3.GetParameter(u1, u2, u3, u4, u5, u6),
				p4.GetParameter(u1, u2, u3, u4, u5, u6),
				p5.GetParameter(u1, u2, u3, u4, u5, u6),
				p6.GetParameter(u1, u2, u3, u4, u5, u6),
				p7.GetParameter(u1, u2, u3, u4, u5, u6));
		}

		template<typename U1, typename U2, typename U3, typename U4, typename U5, typename U6, typename U7>
		ReturnType invoke(U1 u1, U2 u2, U3 u3, U4 u4, U5 u5, U6 u6, U7 u7) {
			return fn(p1.GetParameter(u1, u2, u3, u4, u5, u6, u7),
				p2.GetParameter(u1, u2, u3, u4, u5, u6, u7),
				p3.GetParameter(u1, u2, u3, u4, u5, u6, u7),
				p4.GetParameter(u1, u2, u3, u4, u5, u6, u7),
				p5.GetParameter(u1, u2, u3, u4, u5, u6, u7),
				p6.GetParameter(u1, u2, u3, u4, u5, u6, u7),
				p7.GetParameter(u1, u2, u3, u4, u5, u6, u7));
		}

	protected:
		IncomingFunctionType fn;
		ParamHolder<P1> p1;
		ParamHolder<P2> p2;
		ParamHolder<P3> p3;
		ParamHolder<P4> p4;
		ParamHolder<P5> p5;
		ParamHolder<P6> p6;
		ParamHolder<P7> p7;
	};

	template<typename IncomingFunctionType, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8>
	class Bound_8 : public RefCounter
	{
	public:
		typedef typename BoundHelper<IncomingFunctionType, P1, P2, P3, P4, P5, P6, P7, P8> Helper;
		typedef typename Helper::ReturnType ReturnType;
		typedef typename Helper::OutgoingFunctionType OutgoingFunctionType;

		Bound_8(const IncomingFunctionType &_fn, const P1 &_p1, const P2 &_p2, const P3 &_p3, const P4 &_p4, const P5 &_p5, const P6 &_p6, const P7 &_p7, const P8 &_p8)
			: fn(_fn), p1(_p1), p2(_p2), p3(_p3), p4(_p4), p5(_p5), p6(_p6), p7(_p7), p8(_p8)
		{ }

		ReturnType invoke() {
			return fn(p1.GetParameter(),
				p2.GetParameter(),
				p3.GetParameter(),
				p4.GetParameter(),
				p5.GetParameter(),
				p6.GetParameter(),
				p7.GetParameter(),
				p8.GetParameter());
		}

		template<typename U1>
		ReturnType invoke(U1 u1) {
			return fn(p1.GetParameter(u1),
				p2.GetParameter(u1),
				p3.GetParameter(u1),
				p4.GetParameter(u1),
				p5.GetParameter(u1),
				p6.GetParameter(u1),
				p7.GetParameter(u1),
				p8.GetParameter(u1));
		}

		template<typename U1, typename U2>
		ReturnType invoke(U1 u1, U2 u2) {
			return fn(p1.GetParameter(u1, u2),
				p2.GetParameter(u1, u2),
				p3.GetParameter(u1, u2),
				p4.GetParameter(u1, u2),
				p5.GetParameter(u1, u2),
				p6.GetParameter(u1, u2),
				p7.GetParameter(u1, u2),
				p8.GetParameter(u1, u2));
		}

		template<typename U1, typename U2, typename U3>
		ReturnType invoke(U1 u1, U2 u2, U3 u3) {
			return fn(p1.GetParameter(u1, u2, u3),
				p2.GetParameter(u1, u2, u3),
				p3.GetParameter(u1, u2, u3),
				p4.GetParameter(u1, u2, u3),
				p5.GetParameter(u1, u2, u3),
				p6.GetParameter(u1, u2, u3),
				p7.GetParameter(u1, u2, u3),
				p8.GetParameter(u1, u2, u3));
		}

		template<typename U1, typename U2, typename U3, typename U4>
		ReturnType invoke(U1 u1, U2 u2, U3 u3, U4 u4) {
			return fn(p1.GetParameter(u1, u2, u3, u4),
				p2.GetParameter(u1, u2, u3, u4),
				p3.GetParameter(u1, u2, u3, u4),
				p4.GetParameter(u1, u2, u3, u4),
				p5.GetParameter(u1, u2, u3, u4),
				p6.GetParameter(u1, u2, u3, u4),
				p7.GetParameter(u1, u2, u3, u4),
				p8.GetParameter(u1, u2, u3, u4));
		}

		template<typename U1, typename U2, typename U3, typename U4, typename U5>
		ReturnType invoke(U1 u1, U2 u2, U3 u3, U4 u4, U5 u5) {
			return fn(p1.GetParameter(u1, u2, u3, u4, u5),
				p2.GetParameter(u1, u2, u3, u4, u5),
				p3.GetParameter(u1, u2, u3, u4, u5),
				p4.GetParameter(u1, u2, u3, u4, u5),
				p5.GetParameter(u1, u2, u3, u4, u5),
				p6.GetParameter(u1, u2, u3, u4, u5),
				p7.GetParameter(u1, u2, u3, u4, u5),
				p8.GetParameter(u1, u2, u3, u4, u5));
		}

		template<typename U1, typename U2, typename U3, typename U4, typename U5, typename U6>
		ReturnType invoke(U1 u1, U2 u2, U3 u3, U4 u4, U5 u5, U6 u6) {
			return fn(p1.GetParameter(u1, u2, u3, u4, u5, u6),
				p2.GetParameter(u1, u2, u3, u4, u5, u6),
				p3.GetParameter(u1, u2, u3, u4, u5, u6),
				p4.GetParameter(u1, u2, u3, u4, u5, u6),
				p5.GetParameter(u1, u2, u3, u4, u5, u6),
				p6.GetParameter(u1, u2, u3, u4, u5, u6),
				p7.GetParameter(u1, u2, u3, u4, u5, u6),
				p8.GetParameter(u1, u2, u3, u4, u5, u6));
		}

		template<typename U1, typename U2, typename U3, typename U4, typename U5, typename U6, typename U7>
		ReturnType invoke(U1 u1, U2 u2, U3 u3, U4 u4, U5 u5, U6 u6, U7 u7) {
			return fn(p1.GetParameter(u1, u2, u3, u4, u5, u6, u7),
				p2.GetParameter(u1, u2, u3, u4, u5, u6, u7),
				p3.GetParameter(u1, u2, u3, u4, u5, u6, u7),
				p4.GetParameter(u1, u2, u3, u4, u5, u6, u7),
				p5.GetParameter(u1, u2, u3, u4, u5, u6, u7),
				p6.GetParameter(u1, u2, u3, u4, u5, u6, u7),
				p7.GetParameter(u1, u2, u3, u4, u5, u6, u7),
				p8.GetParameter(u1, u2, u3, u4, u5, u6, u7));
		}

		template<typename U1, typename U2, typename U3, typename U4, typename U5, typename U6, typename U7, typename U8>
		ReturnType invoke(U1 u1, U2 u2, U3 u3, U4 u4, U5 u5, U6 u6, U7 u7, U8 u8) {
			return fn(p1.GetParameter(u1, u2, u3, u4, u5, u6, u7, u8),
				p2.GetParameter(u1, u2, u3, u4, u5, u6, u7, u8),
				p3.GetParameter(u1, u2, u3, u4, u5, u6, u7, u8),
				p4.GetParameter(u1, u2, u3, u4, u5, u6, u7, u8),
				p5.GetParameter(u1, u2, u3, u4, u5, u6, u7, u8),
				p6.GetParameter(u1, u2, u3, u4, u5, u6, u7, u8),
				p7.GetParameter(u1, u2, u3, u4, u5, u6, u7, u8),
				p8.GetParameter(u1, u2, u3, u4, u5, u6, u7, u8));
		}

	protected:
		IncomingFunctionType fn;
		ParamHolder<P1> p1;
		ParamHolder<P2> p2;
		ParamHolder<P3> p3;
		ParamHolder<P4> p4;
		ParamHolder<P5> p5;
		ParamHolder<P6> p6;
		ParamHolder<P7> p7;
		ParamHolder<P8> p8;
	};

	// Bind functions
	template<typename F, typename P1>
	typename Bound_1<typename FuncTraits<F>::FunctionType, P1>::OutgoingFunctionType Bind(const F &fn, const P1 &p1)
 	{
		typedef Bound_1<typename FuncTraits<F>::FunctionType, P1> BoundType;
		BoundType *b = new BoundType(MakeFunction(fn), p1);
		return BoundType::OutgoingFunctionType(b, &BoundType::invoke);
 	}

  	template<typename F, typename P1, typename P2>
	typename Bound_2<typename FuncTraits<F>::FunctionType, P1, P2>::OutgoingFunctionType Bind(const F &fn, const P1 &p1, const P2 &p2)
  	{
		typedef Bound_2<typename FuncTraits<F>::FunctionType, P1, P2> BoundType;
  		BoundType *b = new BoundType(MakeFunction(fn), p1, p2);
 		return BoundType::OutgoingFunctionType(b, &BoundType::invoke);
  	}

	template<typename F, typename P1, typename P2,  typename P3>
	typename Bound_3<typename FuncTraits<F>::FunctionType, P1, P2, P3>::OutgoingFunctionType Bind(const F &fn, const P1 &p1, const P2 &p2, const P3 &p3)
	{
		typedef Bound_3<typename FuncTraits<F>::FunctionType, P1, P2, P3> BoundType;
		BoundType *b = new BoundType(MakeFunction(fn), p1, p2, p3);
		return BoundType::OutgoingFunctionType(b, &BoundType::invoke);
	}

	template<typename F, typename P1, typename P2,  typename P3,  typename P4>
	typename Bound_4<typename FuncTraits<F>::FunctionType, P1, P2, P3, P4>::OutgoingFunctionType Bind(const F &fn, const P1 &p1, const P2 &p2, const P3 &p3, const P4 &p4)
	{
		typedef Bound_4<typename FuncTraits<F>::FunctionType, P1, P2, P3, P4> BoundType;
		BoundType *b = new BoundType(MakeFunction(fn), p1, p2, p3, p4);
		return BoundType::OutgoingFunctionType(b, &BoundType::invoke);
	}

	template<typename F, typename P1, typename P2,  typename P3,  typename P4,  typename P5>
	typename Bound_5<typename FuncTraits<F>::FunctionType, P1, P2, P3, P4, P5>::OutgoingFunctionType Bind(const F &fn, const P1 &p1, const P2 &p2, const P3 &p3, const P4 &p4, const P5 &p5)
	{
		typedef Bound_5<typename FuncTraits<F>::FunctionType, P1, P2, P3, P4, P5> BoundType;
		BoundType *b = new BoundType(MakeFunction(fn), p1, p2, p3, p4, p5);
		return BoundType::OutgoingFunctionType(b, &BoundType::invoke);
	}

	template<typename F, typename P1, typename P2,  typename P3,  typename P4,  typename P5,  typename P6>
	typename Bound_6<typename FuncTraits<F>::FunctionType, P1, P2, P3, P4, P5, P6>::OutgoingFunctionType Bind(const F &fn, const P1 &p1, const P2 &p2, const P3 &p3, const P4 &p4, const P5 &p5, const P6 &p6)
	{
		typedef Bound_6<typename FuncTraits<F>::FunctionType, P1, P2, P3, P4, P5, P6> BoundType;
		BoundType *b = new BoundType(MakeFunction(fn), p1, p2, p3, p4, p5, p6);
		return BoundType::OutgoingFunctionType(b, &BoundType::invoke);
	}

	template<typename F, typename P1, typename P2,  typename P3,  typename P4,  typename P5,  typename P6,  typename P7>
	typename Bound_7<typename FuncTraits<F>::FunctionType, P1, P2, P3, P4, P5, P6, P7>::OutgoingFunctionType Bind(const F &fn, const P1 &p1, const P2 &p2, const P3 &p3, const P4 &p4, const P5 &p5, const P6 &p6, const P7 &p7)
	{
		typedef Bound_7<typename FuncTraits<F>::FunctionType, P1, P2, P3, P4, P5, P6, P7> BoundType;
		BoundType *b = new BoundType(MakeFunction(fn), p1, p2, p3, p4, p5, p6, p7);
		return BoundType::OutgoingFunctionType(b, &BoundType::invoke);
	}

	template<typename F, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8>
	typename Bound_8<typename FuncTraits<F>::FunctionType, P1, P2, P3, P4, P5, P6, P7, P8>::OutgoingFunctionType Bind(const F &fn, const P1 &p1, const P2 &p2, const P3 &p3, const P4 &p4, const P5 &p5, const P6 &p6, const P7 &p7, const P8 &p8)
	{
		typedef Bound_8<typename FuncTraits<F>::FunctionType, P1, P2, P3, P4, P5, P6, P7, P8> BoundType;
		BoundType *b = new BoundType(MakeFunction(fn), p1, p2, p3, p4, p5, p6, p7, p8);
		return BoundType::OutgoingFunctionType(b, &BoundType::invoke);
	}
};


#endif // __DAVA_BIND_H__
