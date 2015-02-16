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

#include "Base/BaseTypes.h"

#if defined(DAVA_MEMORY_PROFILING_ENABLE)

#include <cstdlib>
#include <cassert>

#if defined(__DAVAENGINE_WIN32__)
#include <detours/detours.h>
#elif defined(__DAVAENGINE_ANDROID__) || defined(__DAVAENGINE_MACOS__) || defined(__DAVAENGINE_IPHONE__)
#include <dlfcn.h>
#else
#error "Unknown platform"
#endif

#include "MallocHook.h"
#include "AllocPools.h"
#include "MemoryManager.h"

static void* HookedMalloc(size_t size)
{
    return DAVA::MemoryManager::Allocate(size, DAVA::ALLOC_POOL_APP);
}

static void* HookedRealloc(void* ptr, size_t newSize)
{
    if (nullptr == ptr)
        return malloc(newSize);

    size_t oldSize = DAVA::MemoryManager::BlockSize(ptr);
    if (oldSize > 0)
    {
        void* newPtr = malloc(newSize);
        if (newPtr != nullptr)
        {
            size_t n = oldSize > newSize ? newSize : oldSize;
            memcpy(newPtr, ptr, n);
            free(ptr);
            return newPtr;
        }
        else
            return nullptr;
    }
    else
        return DAVA::MallocHook::Realloc(ptr, newSize);
}

static void* HookedCalloc(size_t count, size_t elemSize)
{
    void* ptr = nullptr;
    if (count > 0 && elemSize > 0)
    {
        ptr = malloc(count * elemSize);
        if (ptr != nullptr)
        {
            memset(ptr, 0, count * elemSize);
        }
    }
    return ptr;
}

static char* HookedStrdup(const char* src)
{
    char* dst = nullptr;
    if (src != nullptr)
    {
        dst = static_cast<char*>(malloc(strlen(src)));
        if (dst != nullptr)
            strcpy(dst, src);
    }
    return dst;
}


static void HookedFree(void* ptr)
{
    DAVA::MemoryManager::Deallocate(ptr);
}

namespace DAVA
{

void* (*MallocHook::RealMalloc)(size_t) = &malloc;
void* (*MallocHook::RealRealloc)(void*, size_t) = &realloc;
void(*MallocHook::RealFree)(void*) = &free;

MallocHook::MallocHook()
{
    Install();
}

void* MallocHook::Malloc(size_t size)
{
    return RealMalloc(size);
}

void* MallocHook::Realloc(void* ptr, size_t newSize)
{
    return RealRealloc(ptr, newSize);
}

void MallocHook::Free(void* ptr)
{
    RealFree(ptr);
}

void MallocHook::Install()
{
#if defined(__DAVAENGINE_WIN32__)
    void* (*realCalloc)(size_t, size_t) = &calloc;
    char* (*realStrdup)(const char*) = &_strdup;

    // TODO: check return values
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    DetourAttach(reinterpret_cast<PVOID*>(&RealMalloc), reinterpret_cast<PVOID>(&HookedMalloc));
    DetourTransactionCommit();

    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    DetourAttach(reinterpret_cast<PVOID*>(&RealRealloc), reinterpret_cast<PVOID>(&HookedRealloc));
    DetourTransactionCommit();

    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    DetourAttach(reinterpret_cast<PVOID*>(&realCalloc), reinterpret_cast<PVOID>(&HookedCalloc));
    DetourTransactionCommit();

    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    DetourAttach(reinterpret_cast<PVOID*>(&realStrdup), reinterpret_cast<PVOID>(&HookedStrdup));
    DetourTransactionCommit();

    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    DetourAttach(reinterpret_cast<PVOID*>(&RealFree), reinterpret_cast<PVOID>(&HookedFree));
    DetourTransactionCommit();

#elif defined(__DAVAENGINE_ANDROID__) || defined(__DAVAENGINE_MACOS__) || defined(__DAVAENGINE_IPHONE__)

    void* handle = reinterpret_cast<void*>(-1);
    void* fptr = nullptr;
    
    fptr = dlsym(handle, "malloc");
    RealMalloc = reinterpret_cast<void* (*)(size_t)>(fptr);
    assert(fptr != nullptr);
    
    fptr = dlsym(handle, "free");
    RealFree = reinterpret_cast<void (*)(void*)>(fptr);
    assert(fptr != nullptr);
    
#endif
}

}   // namespace DAVA

#if defined(__DAVAENGINE_ANDROID__) || defined(__DAVAENGINE_MACOS__) || defined(__DAVAENGINE_IPHONE__)

void* malloc(size_t size)
{
    return HookedMalloc(size);
}

void free(void* ptr)
{
    HookedFree(ptr);
}

void* realloc(void* ptr, size_t newSize)
{
    return HookedRealloc(ptr, newSize);
}

void* calloc(size_t count, size_t elemSize)
{
    return HookedCalloc(count, elemSize);
}

char* strdup(const char *src)
{
    return HookedStrdup(src);
}

#endif  // defined(__DAVAENGINE_ANDROID__) || defined(__DAVAENGINE_MACOS__) || defined(__DAVAENGINE_IPHONE__)

#endif  // defined(DAVA_MEMORY_PROFILING_ENABLE)
