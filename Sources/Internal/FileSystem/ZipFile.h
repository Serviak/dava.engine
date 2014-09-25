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


#ifndef __DAVAENGINE_APK_FILE_H__
#define __DAVAENGINE_APK_FILE_H__

#include "Base/BaseTypes.h"

#if defined(__DAVAENGINE_ANDROID__)

#include "FileSystem/DynamicMemoryFile.h"

namespace DAVA
{

class Mutex;

class ZipFile: public DynamicMemoryFile
{

public:
    static File* CreateFromAPK(const FilePath &filePath, uint32 attributes);
#ifdef USE_LOCAL_RESOURCES
    static File* CreateFromZip(const FilePath &filePath, uint32 attributes);
#endif

private:
    
    ZipFile();
    virtual ~ZipFile();

    static ZipFile* CreateFromPath(zip* package, const FilePath &filePath, const String &path, uint32 attributes);
    static ZipFile* CreateFromData(const FilePath &filePath, const uint8 * data, int32 dataSize, uint32 attributes);

    static Mutex mutex;

#ifdef USE_LOCAL_RESOURCES
    static zip* exZipPackage;
#endif
};


};


#endif // __DAVAENGINE_ANDROID__

#endif //__DAVAENGINE_APK_FILE_H__
