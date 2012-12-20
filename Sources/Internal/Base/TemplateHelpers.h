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
#ifndef __DAVAENGINE_TEMPLATEHELPERS_H__
#define __DAVAENGINE_TEMPLATEHELPERS_H__

//#include "Base/BaseTypes.h"
//#include "Base/BaseObjectChecker.h"
//#include "Debug/DVAssert.h"
//#include "DAVAConfig.h"
//#include "Base/RefPtr.h"
//#include "Render/RenderBase.h"
#include <typeinfo>

namespace DAVA
{

	// Alexandresky style compile time assertion. 
template <bool> struct CompileTimeError;
template <> struct CompileTimeError<true> {};
#define COMPILER_ASSERT(expr)  (DAVA::CompileTimeError<(expr)!=0>());




template <int v>
class Int2Type
{
    enum {value = v };
};
    
template <class T>
class Type2Type
{
    typedef T OriginalType;
};
    
    
template <bool flag, typename T, typename U>
struct Select
{
    typedef T Result;
};
template <typename T, typename U>
struct Select<false, T, U>
{
    typedef U Result;
};
    
    
template <class TO, class FROM>
class Conversion
{
    typedef char Small;
    struct Big { char value[2]; };
    
    static Small Test(FROM);
    static Big Test(...);
    
    static TO MakeTO();
    
public:
    enum 
    {
        exists = 
        sizeof(Test(MakeTO())) == sizeof(Small)
    };
    
    enum 
    {
        sameType = false
    };
};   
    
template <class T>
class Conversion<T, T>
{
public:
    enum {exists = true };
    enum {sameType = true };
};
    
#define SUPERSUBCLASS(SUPER, SUB) (Conversion<const SUB*, const SUPER*>::exists && !Conversion<const SUPER*, const void*>::sameType) 

class NullType{};
struct EmptyType{};
    
template<typename U>
struct PointerTraits
{
    enum{result = false };
    typedef NullType PointeeType;
};
template <typename U>
struct PointerTraits<U*>
{
    enum{result = true };
    typedef U PointeeType;
};

template<typename U>
struct ReferenceTraits
{
    enum{result = false };
    typedef NullType ReferenceType;
};
template <typename U>
struct ReferenceTraits<U&>
{
    enum{result = true };
    typedef U ReferenceType;
};

template<class U>
struct P2MTraits
{
    enum{result = false };
};
template <class R, class V>
struct P2MTraits<R V::*>
{
    enum{result = true };
};    

template <typename T>
class TypeTraits
{
public:    
    enum {isPointer = PointerTraits<T>::result };    
    enum {isReference = ReferenceTraits<T>::result };   
    enum {isPointerToMemberFunction = P2MTraits<T>::result };
    
};

    
    /**
     \brief Works like dynamic_cast for Debug and like a static_cast for release.
     */
    template<class C, class O>
    C DynamicTypeCheck(O* pObject)
    {
#ifdef DAVA_DEBUG
        C c = dynamic_cast<C>(pObject);
        if (!c)
        {//assert emulation )
            int i = 0;
            *((int*)i) = 0;
        }
        return c;
#else
        return static_cast<C>(pObject);
#endif
    }
    
    /**
     \brief Returns true if object pointer is a pointer to the exact class.
     */
    template<class C, class O>
    bool IsPointerToExactClass(const O* pObject) 
    {
		if (pObject)
        {
			COMPILER_ASSERT(!TypeTraits<C>::isPointer);//You should not use pointers for this method
			return &typeid(*pObject) == &typeid(C);
		}
	    return false;
    }
    
//    template<class C, class O>
//    C CastExactClass(const O* pObject)
//    {
//		if (pObject)
//        {
//			COMPILER_ASSERT(TypeTraits<C>::isPointer);//You should not use pointers for this method
//			if (&typeid(*pObject) == &typeid(C));
//            {
//                return (C)pObject;
//            }
//		}
//	    return 0;
//    }
    
    /* TEST, need to transfer to unit tests.
     Logger::Debug("%d", Conversion<double, int>::exists);
     Logger::Debug("%d", Conversion<Component*, VisibilityAABBoxComponent*>::exists);
     Logger::Debug("%d", Conversion<VisibilityAABBoxComponent*, Component*>::exists);
     Logger::Debug("%d", SUPERSUBCLASS(VisibilityAABBoxComponent, Component));
     Logger::Debug("%d", SUPERSUBCLASS(Component, VisibilityAABBoxComponent));
     Logger::Debug("%d", SUPERSUBCLASS(void*, VisibilityAABBoxComponent));
     Logger::Debug("%d", SUPERSUBCLASS(BaseObject, VisibilityAABBoxComponent));
     
     
     Logger::Debug("(BaseObject*) isPointer: %d", TypeTraits<BaseObject*>::isPointer);
     Logger::Debug("(BaseObject*) isReference: %d", TypeTraits<BaseObject*>::isReference);
     Logger::Debug("(BaseObject*) isPointerToMemberFunction: %d", TypeTraits<BaseObject*>::isPointerToMemberFunction);
     
     Logger::Debug("(BaseObject&) isPointer: %d", TypeTraits<BaseObject&>::isPointer);
     Logger::Debug("(BaseObject&) isReference: %d", TypeTraits<BaseObject&>::isReference);
     Logger::Debug("(BaseObject&) isPointerToMemberFunction: %d", TypeTraits<BaseObject&>::isPointerToMemberFunction);
     
     void(VisibilityAABBoxSystem::*func)() = &VisibilityAABBoxSystem::Run;
     
     //    Logger::Debug("(&VisibilityAABBoxSystem::Run) isPointer: %d", TypeTraits<VisibilityAABBoxSystem::Run>::isPointer);
     //    Logger::Debug("(&VisibilityAABBoxSystem::Run) isReference: %d", TypeTraits<&VisibilityAABBoxSystem::Run>::isReference);
     //    Logger::Debug("(&VisibilityAABBoxSystem::Run) isPointerToMemberFunction: %d", TypeTraits<&VisibilityAABBoxSystem::Run>::isPointerToMemberFunction);
     */    

    
    
};


#endif // __DAVAENGINE_TEMPLATEHELPERS_H__

