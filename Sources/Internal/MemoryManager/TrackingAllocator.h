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

#ifndef __DAVAENGINE_MEMORYMANAGERALLOCATOR_H__
#define __DAVAENGINE_MEMORYMANAGERALLOCATOR_H__

#include "Base/BaseTypes.h"

#if defined(DAVA_MEMORY_PROFILING_ENABLE)

#include <cstddef>
#include <limits>

#include "MemoryManager/AllocatorBridge.h"

namespace DAVA
{

// C++11 introduced std::allocator_traits which helps in defining custom allocators
// But it doesn't works in MSVC on some data structures:
//  using mystr = std::basic_string<char, std::char_traits<char>, my_allocator<char>>;
//  std::vector<mystr, my_allocator<mystr>> v;

// Allocator for application memory allocations, parameter AllocPoolT specifies memory pool allocated memory belongs to
template<typename T, unsigned int AllocPoolT>
class TrackingAllocator
{
public:
    using value_type = T;
    using pointer = T*;
    using const_pointer = const T*;
    using reference = T&;
    using const_reference = const T&;
    using size_type = size_t;
    using difference_type = ptrdiff_t;

    template <typename U>
    struct rebind
    {
        typedef TrackingAllocator<U, AllocPoolT> other;
    };

    TrackingAllocator() = default;
    TrackingAllocator(const TrackingAllocator&) = default;
    template <typename U, unsigned int AllocPoolU>
    TrackingAllocator(const TrackingAllocator<U, AllocPoolU>&) DAVA_NOEXCEPT{}
    ~TrackingAllocator() = default;

    size_type max_size() const DAVA_NOEXCEPT
    {
        return std::numeric_limits<size_type>::max() / sizeof(T);
    }

    pointer address(reference ref) const DAVA_NOEXCEPT
    {
        return std::addressof(ref);
    }

    const_pointer address(const_reference cref) const DAVA_NOEXCEPT
    {
        return std::addressof(cref);
    }

    pointer allocate(size_type n, std::allocator<void>::const_pointer hint = 0)
    {
        void* ptr = TrackingAlloc(n * sizeof(T), AllocPoolT);
        if (ptr != nullptr)
        {
            return static_cast<pointer>(ptr);
        }
        throw std::bad_alloc();
    }

    void deallocate(pointer ptr, size_type n)
    {
        TrackingDealloc(ptr);
    }

    void construct(pointer ptr, const_reference value)
    {
        ::new (static_cast<void*>(ptr)) T(value);
    }

    template<typename U, typename... Args>
    void construct(U* ptr, Args&&... args)
    {
        ::new (static_cast<void*>(ptr)) U(std::forward<Args>(args)...);
    }

    void destroy(pointer ptr)
    {
        ptr->~T();
    }

    template<typename U>
    void destroy(U* ptr)
    {
        ptr->~U();
    }
};

//////////////////////////////////////////////////////////////////////////
template<typename T1, unsigned int AllocPoolT1, typename T2, unsigned int AllocPoolT2>
inline bool operator == (const TrackingAllocator<T1, AllocPoolT1>&, const TrackingAllocator<T2, AllocPoolT2>&)
{
    return true;    // TrackingAllocator is stateless so two allocators are always equal
}

template<typename T1, unsigned int AllocPoolT1, typename T2, unsigned int AllocPoolT2>
inline bool operator != (const TrackingAllocator<T1, AllocPoolT1>&, const TrackingAllocator<T2, AllocPoolT2>&)
{
    return false;
}

}   // namespace DAVA

#endif  // defined(DAVA_MEMORY_PROFILING_ENABLE)

#endif  // __DAVAENGINE_MEMORYMANAGERALLOCATOR_H__
