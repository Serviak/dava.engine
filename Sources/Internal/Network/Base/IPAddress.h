/*==================================================================================
    Copyright(c) 2008, binaryzebra
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

#ifndef __DAVAENGINE_IPADDRESS_H__
#define __DAVAENGINE_IPADDRESS_H__

#include <libuv/uv.h>

#include <Base/BaseTypes.h>

namespace DAVA
{

class IPAddress
{
public:
    IPAddress(uint32 address = 0) : addr(htonl(address)) {}

    uint32 ToUInt() const;

    bool IsUnspecified() const;
    bool IsMulticast() const;

    bool ToString(char8* buffer, std::size_t size) const;
    String ToString() const;

    static IPAddress FromString(const char8* addr);

private:
    uint32 addr;
};

//////////////////////////////////////////////////////////////////////////
inline uint32 IPAddress::ToUInt() const
{
    return ntohl(addr);
}

inline bool IPAddress::IsUnspecified() const
{
    return 0 == addr;
}

inline bool IPAddress::IsMulticast() const
{
    return 0xE0000000 == (ToUInt() & 0xF0000000);
}

}	// namespace DAVA

#endif  // __DAVAENGINE_IPADDRESS_H__
