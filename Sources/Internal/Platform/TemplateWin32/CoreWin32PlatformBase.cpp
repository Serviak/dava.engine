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


#include "Platform/TemplateWin32/CorePlatformWin32.h"
#include "Platform/TemplateWin32/WindowsSpecifics.h"
#include "Platform/Thread.h"
#include "Utils/Utils.h"

#if defined(__DAVAENGINE_WIN32__)

#include <shellapi.h>

namespace DAVA {

CoreWin32PlatformBase::CoreWin32PlatformBase() :
    hWindow(0),
    hInstance(0)
{
}

HINSTANCE CoreWin32PlatformBase::GetInstance() const
{
    return hInstance;
}

HWND CoreWin32PlatformBase::GetWindow() const
{
    return hWindow;
}

void CoreWin32PlatformBase::InitArgs()
{
    LPWSTR *szArglist;
    int nArgs;
    int i;
    szArglist = ::CommandLineToArgvW(::GetCommandLineW(), &nArgs);
    if( NULL == szArglist )
    {
        Logger::Error("CommandLineToArgvW failed\n");
        return;
    }
    else
    {
        Vector<String> & cl = GetCommandLine();
        for( i=0; i<nArgs; i++)
        {
            WideString w = szArglist[i];
            String nonWide = WStringToString(w);
            cl.push_back(nonWide);
            Logger::FrameworkDebug("%d: %s\n", i, nonWide.c_str());
        }
    }
    // Free memory allocated for CommandLineToArgvW arguments.
    LocalFree(szArglist);
}

void CoreWin32PlatformBase::Quit()
{
    PostQuitMessage(0);
}

}

#endif // #if defined(__DAVAENGINE_WIN32__)