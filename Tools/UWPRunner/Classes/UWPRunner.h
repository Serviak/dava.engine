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


#ifndef UWP_RUNNER_H
#define UWP_RUNNER_H

#include "Base/BaseTypes.h"
#include "FileSystem/File.h"
#include "FileSystem/FilePath.h"
#include "Network/NetCore.h"
#include "UWPLogConsumer.h"
#include "AppxBundleHelper.h"

struct PackageOptions
{
    //TODO: replace on Optional
    DAVA::String mainPackage;
    DAVA::String packageToInstall;
    DAVA::String architecture;
    DAVA::String profile;
    DAVA::String dependencies;
    DAVA::String outputFile;
    DAVA::Vector<DAVA::String> resources;
    bool useTeamCityTestOutput = false;
    bool installOnly = false;
    bool runOnly = false;
    bool isDavaApplication = false;
};
PackageOptions ParseCommandLine();
bool CheckOptions(const PackageOptions& options);

class Runner;
class RegKey;

class UWPRunner
{
public:
    UWPRunner(const PackageOptions& opt);
    ~UWPRunner();

    void Run();
    bool IsSucceed();

private:
    void Run(Runner& runner);
    void WaitApp();

    void ProcessPackageOptions();
    void ProcessBundlePackage();
    void ProcessProfileInfo();
    void InitializeNetwork(bool isMobileDevice);
    void UnInitializeNetwork();

    bool UpdateIpOverUsbConfig(RegKey& key);
    bool ConfigureIpOverUsb();
    bool RestartIpOverUsb();

    void NetLogOutput(const DAVA::String& logString);

    PackageOptions options;
    DAVA::Signal<> cleanNeeded;
    std::unique_ptr<AppxBundleHelper> bundleHelper;
    UWPLogConsumer logConsumer;
    DAVA::SigConnectionID logConsumerConnectionID = DAVA::SigConnectionID();
    DAVA::Net::NetCore::TrackId controllerId = DAVA::Net::NetCore::INVALID_TRACK_ID;
    DAVA::String qtProfile;
    DAVA::RefPtr<DAVA::File> outputFile;
    bool succeed = false;
    bool davaApplicationTerminated = false;
};

#endif // UWP_RUNNER_H