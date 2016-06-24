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


#pragma once

#include "Infrastructure/BaseScreen.h"
#include <FileSystem/FilePath.h>
#include <PackManager/PackManager.h>

class PackManagerTest : public BaseScreen, DAVA::UITextFieldDelegate
{
public:
    PackManagerTest();

private:
    void TextFieldOnTextChanged(DAVA::UITextField* textField, const DAVA::WideString& newText, const DAVA::WideString& /*oldText*/) override;
    void UpdateDescription();

    void LoadResources() override;
    void UnloadResources() override;

    void OnStartInitializeClicked(DAVA::BaseObject* sender, void* data, void* callerData);
    void OnStartDownloadClicked(DAVA::BaseObject* sender, void* data, void* callerData);
    void OnStartStopLocalServerClicked(DAVA::BaseObject* sender, void* data, void* callerData);
    void OnCheckFileClicked(DAVA::BaseObject* sender, void* data, void* callerData);

    void OnPackStateChange(const DAVA::PackManager::Pack& pack);
    void OnPackDownloadChange(const DAVA::PackManager::Pack& pack);
    void OnRequestChange(const DAVA::PackManager::IRequest& request);
    void OnInitChange(PackManager::IInit& init);

    DAVA::String sqliteDbFile = "db_{gpu}.db.zip";
    DAVA::FilePath folderWithDownloadedPacks = "~doc:/PackManagerTest/packs/";
    DAVA::FilePath readOnlyDirWithPacks = "~res:/TestData/PackManagerTest/packs/read_only_packs/";
    // TODO quick and dirty way to test download on all platforms, in future replace with local http server
    DAVA::String urlToServerSuperpack = "http://by1-builddlc-01.corp.wargaming.local/DLC_Blitz/packs/superpack.dvpk"; //"http://127.0.0.1:2424/packs/common/";
    DAVA::String urlPacksGpu = "http://by1-builddlc-01.corp.wargaming.local/DLC_Blitz/packs/{gpu}/"; //"http://127.0.0.1:2424/packs/{gpu}/";
    DAVA::String gpuArchitecture;

    DAVA::UIStaticText* packNameLoading = nullptr;
    DAVA::UIButton* startLoadingButton = nullptr;

    DAVA::UIButton* startServerButton = nullptr;
    DAVA::UIButton* stopServerButton = nullptr;

    DAVA::UIControl* progressBar = nullptr;
    DAVA::UITextField* packInput = nullptr;
    DAVA::UIControl* redControl = nullptr;
    DAVA::UIControl* greenControl = nullptr;
    DAVA::UIStaticText* description = nullptr;
    DAVA::UITextField* url = nullptr;
    DAVA::UITextField* filePathField = nullptr;
    DAVA::UIButton* checkFile = nullptr;
    DAVA::UIButton* startInit = nullptr;
};
