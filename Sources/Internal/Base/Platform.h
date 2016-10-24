#ifndef __DAVAENGINE_PLATFORM__
#define __DAVAENGINE_PLATFORM__

#include "DAVAConfig.h"

//-------------------------------------------------------------------------------------
//Compiler features
//-------------------------------------------------------------------------------------
//GCC && Clang
#if defined(__GNUC__)

#define DAVA_NOINLINE __attribute__((noinline))
#define DAVA_FORCEINLINE inline __attribute__((always_inline))
#define DAVA_ALIGNOF(x) alignof(x)
#define DAVA_CONSTEXPR constexpr
#define DAVA_DEPRECATED(func) func __attribute__((deprecated))
#define DAVA_ALIGNED(Var, Len) Var __attribute__((aligned(Len)))
#define DAVA_NOEXCEPT noexcept

//Microsoft Visual C++
#elif defined(_MSC_VER)

#define DAVA_NOINLINE __declspec(noinline)
#define DAVA_FORCEINLINE __forceinline
#define DAVA_ALIGNOF(x) __alignof(x)

#if _MSC_VER >= 1900 //msvs 2015 RC or later
//Constexpr is not supported even in VS2013 (partially supported in 2015 CTP)
#define DAVA_CONSTEXPR constexpr
#define DAVA_NOEXCEPT noexcept
#else
#define DAVA_CONSTEXPR
#define DAVA_NOEXCEPT throw()
#endif

#define DAVA_DEPRECATED(func) __declspec(deprecated) func
#define DAVA_ALIGNED(Var, Len) __declspec(align(Len)) Var

#endif

// clang-format off
//detecting of compiler features definitions
#if !defined(DAVA_NOINLINE) || \
    !defined(DAVA_FORCEINLINE) || \
    !defined(DAVA_ALIGNOF) || \
    !defined(DAVA_NOEXCEPT) || \
    !defined(DAVA_CONSTEXPR) || \
    !defined(DAVA_DEPRECATED) || \
    !defined(DAVA_ALIGNED)
#error Some compiler features is not defined for current platform
#endif
// clang-format on

#if defined(__clang__)
#define DAVA_SWITCH_CASE_FALLTHROUGH [[clang::fallthrough]]
#else
#define DAVA_SWITCH_CASE_FALLTHROUGH
#endif

//suppressing of deprecated functions
#ifdef DAVAENGINE_HIDE_DEPRECATED
#undef DAVA_DEPRECATED
#define DAVA_DEPRECATED(func) func
#endif

//-------------------------------------------------------------------------------------
//Platform detection
//-------------------------------------------------------------------------------------
//Detection of Apple
#if defined(__GNUC__) && \
(defined(__APPLE_CPP__) || defined(__APPLE_CC__) || defined(__MACOS_CLASSIC__))

#include <AvailabilityMacros.h>
#include <TargetConditionals.h>
#include <mach/mach.h>
#include <mach/mach_time.h>
#include <unistd.h>
#endif

//Detection of Windows
#if defined(__DAVAENGINE_WINDOWS__)

//Platform defines
#define __DAVASOUND_AL__
#define WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
#define NOMINMAX // undef macro min and max from windows headers
#endif

#include <Windows.h>
#include <Windowsx.h>

#undef DrawState
#undef GetCommandLine
#undef GetClassName
#undef Yield
#undef ERROR
#undef DELETE

//Detection of windows platform type
#if !defined(WINAPI_FAMILY_PARTITION) || WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
#define __DAVAENGINE_WIN32__
#elif WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_APP)
#define __DAVAENGINE_WIN_UAP__
#define __DAVAENGINE_WIN_UAP_INCOMPLETE_IMPLEMENTATION__MARKER__
#define __DAVAENGINE_WIN_UAP_INCOMPLETE_IMPLEMENTATION__ DVASSERT_MSG(false, "Feature has no implementation or partly implemented")
#endif

//Using C++11 concurrency as default
#if defined(__DAVAENGINE_WIN_UAP__) && !defined(USE_CPP11_CONCURRENCY)
#define USE_CPP11_CONCURRENCY
#endif

#endif

#if defined(__DAVAENGINE_WINDOWS__)
#define Snprintf _snprintf
#else
#define Snprintf snprintf
#endif

#endif // __DAVAENGINE_PLATFORM__