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


#ifndef __DLC_DOWNLOAD_TEST_H__
#define __DLC_DOWNLOAD_TEST_H__

#include "TestTemplate.h"
#include "DLC/Downloader/CurlDownloader.h"

using namespace DAVA;

class CurlTestDownloader : public CurlDownloader
{
public:
    CurlTestDownloader(uint32 operationTimeout = 1000);
protected:
    virtual size_t SaveData(void *ptr, size_t size, size_t nmemb);
};


class DLCDownloadTest: public TestTemplate<DLCDownloadTest>
{
public:
    DLCDownloadTest();

    virtual void LoadResources();
    virtual void UnloadResources();

    void DownloadCallback(const uint32 &taskId, const DownloadStatus &status);
    void WaitForTaskState(const uint32 &taskId, const DownloadStatus &status, const uint32 timeout);

    String StorePathForUrl(const String &url);
    void TestFunction(PerfFuncData *data);

private:
    const String serverUrl;
    const String testFileEmpty;
    const String testFileOne;
    uint32 taskIdToWait;
    DownloadStatus statusToWait;
};

#endif
