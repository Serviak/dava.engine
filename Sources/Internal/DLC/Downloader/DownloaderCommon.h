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

#ifndef __DOWNLOADER_COMMON_H__
#define __DOWNLOADER_COMMON_H__

#include "Base/BaseTypes.h"
#include "FileSystem/FilePath.h"

namespace DAVA
{

enum DownloadType
{
    RESUMED = 0,
    FULL,
    GET_SIZE,
};


enum DownloadStatus
{
    DL_PENDING = 0,
    DL_IN_PROGRESS,
    DL_FINISHED,
    DL_UNKNOWN,
};
        
enum DownloadError
{
    DLE_NO_ERROR = 0,
    DLE_CANCELLED,
    DLE_CANNOT_RESUME,
    DLE_COULDNT_RESOLVE_HOST,
    DLE_CANNOT_CONNECT,
    DLE_CONTENT_NOT_FOUND,
    DLE_COMMON_ERROR,
    DLE_INIT_ERROR,        
    DLE_FILE_ERROR,
    DLE_UNKNOWN,
};

struct DownloadTaskDescription
{
    DownloadTaskDescription(const String &srcUrl, const FilePath &storeToFilePath, DownloadType downloadMode, int32 _timeout, int32 _retriesCount, char8 _partsCount);

    uint32 id;
    String url;
    FilePath storePath;
    int32 timeout;
    int32 retriesCount;
    int32 retriesLeft;
    DownloadType type;
    DownloadStatus status;
    DownloadError error;
    uint64 downloadTotal;
    uint64 downloadProgress;
    char8 partsCount;
};

class Downloader;
struct PartInfo
{
    Downloader *downloader;
    uint64 seekPos;
    uint64 size;
    uint64 progress;
};

}

#endif