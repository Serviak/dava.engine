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

#ifndef __DAVAENGINE_DLC_H__
#define __DAVAENGINE_DLC_H__

#include "Base/BaseTypes.h"
#include "Platform/Thread.h"
#include "Downloader/DownloaderCommon.h"

namespace DAVA
{

class DLC
{
public:
    enum DLCError
    {
        DE_NO_ERROR = 0,

        DE_WAS_CANCELED,
        DE_INIT_ERROR,
        DE_CHECK_ERROR,
        DE_DOWNLOAD_ERROR,
        DE_PATCH_ERROR_LITE,
        DE_PATCH_ERROR_FULL
    };

    enum DLCState
    {
        DS_INIT,
        DS_CHECKING_INFO,
        DS_CHECKING_PATCH,
        DS_READY,
        DS_DOWNLOADING,
        DS_PATCHING,
        DS_CANCELLING,
        DS_CLEANING,
        DS_DONE
    };

    /**
        \brief Create DLC state machine, that will check or apply patch from given URL.
        \param[in] dlcUrl - remote server url.
        \param[in] sourceDir - local directory, containing original files, that should be patched.
        \param[in] desrinationDir - local directory, where patched files should be stored. Can be the same as sourceDir.
        \param[in] workingDir - local directory, where DLC temporary files should be stored.
        \param[in] gVersion - current game version. Depending on this parameter, will be searched DLC info-file on a given (by URL parameter) remote server.
        \param[in] forceFullUpdate - "true" value will force full-patch to be downloaded from the server. "false" leaves patch version to be determined automatically.
    */
    DLC(const String &dclUrl, const FilePath &sourceDir, const FilePath &destinationDir, const FilePath &workingDir, uint32 gVersion, bool forceFullUpdate = false);
    ~DLC();

    /**
		\brief Starts DLC check-executing process, which will continue until the information from the remote server is received. 
        DLC state machine will go into the READY state if update is required and there were no error, or in DONE state if no updates are required or there were some error.
    */
    void Check();

    /**
		\brief Starts DLC full-executing process. DLC state machine will go into the DONE state, when process finished with or without errors.
    */
    void Start();

    /**
        \brief Stops the current DLC process. DLC state machine will go into the DONE state, with error DE_WAS_CANCELED
    */
    void Cancel();

    /**
        \brief Returns the progress for current DLC state.
        \param[out] cur - current progress value
        \param[out] total - expected total progress value
    */
    void GetProgress(uint64 &cur, uint64 &total) const;

    /**
        \brief Returns current DLC state.
    */
    DLCState GetState() const;

    /**
        \brief Returns DLC state machine error.
    */
    DLCError GetError() const;
    
protected:
    enum DLCEvent
    {
        EVENT_CANCEL,
        EVENT_CHECK_START,
        EVENT_CHECK_ONLY,
        EVENT_CHECK_OK,
        EVENT_CHECK_ERROR,
        EVENT_DOWNLOAD_START,
        EVENT_DOWNLOAD_OK,
        EVENT_DOWNLOAD_ERROR,
        EVENT_PATCH_START,
        EVENT_PATCH_OK,
        EVENT_PATCH_ERROR_LITE,
        EVENT_PATCH_ERROR_FULL,
        EVENT_CLEAN_OK,
        EVENT_CANCEL_OK
    };
    
    struct DLCContext
    {
        String remoteUrl;
        uint32 localVer;
        bool forceFullUpdate;

        FilePath localWorkingDir;
        FilePath localSourceDir;
        FilePath localDestinationDir;
        FilePath localVerStorePath;

        String remoteVerUrl;
        uint32 remoteVer;
        uint32 remoteVerDownloadId;
        uint32 remoteFullSizeDownloadId;
        uint32 remoteLiteSizeDownloadId;
        FilePath remoteVerStotePath;

        String remotePatchFullUrl;
        String remotePatchLiteUrl;

        String remotePatchUrl;
        uint64 remotePatchSize;
        uint32 remotePatchDownloadId;
        FilePath remotePatchStorePath;

        uint32 patchCount;
        uint32 patchIndex;
        bool patchingOk;

        FilePath stateInfoStorePath;
        FilePath downloadInfoStorePath;
        uint32 prevState;
    };

    DLCState dlcState;
    DLCError dlcError;
    DLCContext dlcContext;

    // FSM variables
    bool fsmAutoReady;

    // patch thread variables
    Thread *patchingThread;

    void PostEvent(int event);
    void PostEventJob(BaseObject *caller, void *callerData, void *userData);
    void FSM(int event);

    void StepCheckInfoBegin();
    void StepCheckInfoFinish(const uint32 &id, const DownloadStatus &status);
    void StepCheckInfoCancel();

    void StepCheckPatchBegin();
    void StepCheckPatchFinish(const uint32 &id, const DownloadStatus &status);
    void StepCheckPatchCancel();

    void StepDownloadPatchBegin();
    void StepDownloadPatchFinish(const uint32 &id, const DownloadStatus &status);
    void StepDownloadPatchCancel();

    void StepPatchBegin();
    void StepPatchFinish(BaseObject *caller, void *callerData, void *userData);
    void StepPatchCancel();

    void StepClean();
    void StepDone();

    void PatchingThread(BaseObject *caller, void *callerData, void *userData);

    // helper functions
    uint32 ReadUint32(const FilePath &path);
    void WriteUint32(const FilePath &path, uint32 value);

    String MakePatchUrl(uint32 localVer, uint32 removeVer);
};

}

#endif // __DAVAENGINE_DLC_H__
