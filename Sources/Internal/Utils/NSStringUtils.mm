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


#include "NSStringUtils.h"

#if defined(__DAVAENGINE_MACOS__) || defined(__DAVAENGINE_IPHONE__)

namespace DAVA
{
NSString* NSStringFromString(const DAVA::String& str)
{
    NSStringEncoding encoding = CFStringConvertEncodingToNSStringEncoding(kCFStringEncodingASCII);
    NSString* nsstring = [[[NSString alloc] initWithBytes:str.c_str()
                                                   length:str.length()
                                                 encoding:encoding] autorelease];
    return nsstring;
}

NSString* NSStringFromWideString(const DAVA::WideString& str)
{
    NSStringEncoding encoding = CFStringConvertEncodingToNSStringEncoding(kCFStringEncodingUTF32LE);
    NSString* nsstring = [[[NSString alloc] initWithBytes:str.c_str()
                                                   length:str.length() * sizeof(wchar_t)
                                                 encoding:encoding] autorelease];
    return nsstring;
}

String StringFromNSString(NSString* string)
{
    if (string)
    {
        return String([string cStringUsingEncoding:NSASCIIStringEncoding]);
    }
    else
    {
        return "";
    }
}

WideString WideStringFromNSString(NSString* string)
{
    if (string)
    {
        NSStringEncoding encoding = CFStringConvertEncodingToNSStringEncoding(kCFStringEncodingUTF32LE);
        NSData* data = [string dataUsingEncoding:encoding];
        return WideString(static_cast<const wchar_t*>(data.bytes), data.length / sizeof(wchar_t));
    }
    else
    {
        return L"";
    }
}
}

#endif //#if defined (__DAVAENGINE_MACOS__) || defined (__DAVAENGINE_IPHONE__)
