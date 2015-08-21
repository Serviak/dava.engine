﻿/*==================================================================================
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


#include "Utils/UTF8Utils.h"
#include "FileSystem/Logger.h"
#include "Debug/DVAssert.h"

#if defined(__DAVAENGINE_WINDOWS__)

#include <Windows.h>

namespace DAVA
{

void  UTF8Utils::EncodeToWideString(const uint8 * string, size_t size, WideString & resultString)
{
	resultString = L"";

	int32 wstringLen = MultiByteToWideChar(CP_UTF8, 0, (LPCSTR)string, static_cast<int>(size), NULL, NULL);
	if (!wstringLen)
	{
		return;
	}

	wchar_t* buf = new wchar_t[wstringLen];
	int32 convertRes = MultiByteToWideChar(CP_UTF8, 0, (LPCSTR)string, static_cast<int>(size), buf, wstringLen);
	if (convertRes)
	{
		resultString = WideString(buf, wstringLen);
	}

	delete[] buf;
};

String UTF8Utils::EncodeToUTF8(const WideString& wstring)
{
    return EncodeToUTF8(wstring.c_str());
};

String UTF8Utils::EncodeToUTF8(const wchar_t* wideString)
{
    DVASSERT(wideString != nullptr);

    String result;
    // Note: WideCharToMultiByte makes room for zero terminator in resulting string if wideString length is set to -1
    int bufSize = WideCharToMultiByte(CP_UTF8, 0, wideString, -1, 0, 0, nullptr, nullptr);
    if (bufSize > 0)
    {
        result.resize(bufSize);
        WideCharToMultiByte(CP_UTF8, 0, wideString, -1, &*result.begin(), bufSize, nullptr, nullptr);
        result.pop_back();  // Get rid of extra zero terminator appended by WideCharToMultiByte
    }
    return result;
}

}   // namespace DAVA

#endif
