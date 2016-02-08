/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/


#ifndef __LOGENGINE_TYPES_H__
#define __LOGENGINE_TYPES_H__

#include <string>
#include <list>
#include <map>
#include <vector>
#include <algorithm>

namespace Log
{
typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int uint32;
typedef unsigned long long uint64;

typedef signed char int8;
typedef signed short int16;
typedef signed int int32;
typedef signed long long int64;

typedef char char8;
typedef short char16;

typedef float float32;
typedef double float64;

typedef std::string String;

//template <typename _Ty, typename _Ax = std::allocator(_Ty)>
//class List : public std::list<_Ty, _Ax>  {};


#define List std::list
#define Vector std::vector

template <class _Kty,
          class _Ty,
          class _Pr = std::less<_Kty>,
          class _Alloc = std::allocator<std::pair<const _Kty, _Ty>>>

class Map : public std::map<_Kty, _Ty, _Pr, _Alloc>
{
};



#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif

template <class T>
inline T Min(T a, T b)
{
    return (a < b) ? (a) : (b);
}

template <class T>
inline T Max(T a, T b)
{
    return (a > b) ? (a) : (b);
}


#define Memcpy memcpy
#define Memset memset

template <class TYPE>
void SafeDelete(TYPE*& d)
{
    if (d)
    {
        delete d;
        d = 0;
    }
}

template <class TYPE>
void SafeDeleteArray(TYPE*& d)
{
    if (d)
    {
        delete[] d;
        d = 0;
    }
}
};

using namespace Log;
//#include "LogEngineConfig.h"
//#include "Assert.h"


#endif
