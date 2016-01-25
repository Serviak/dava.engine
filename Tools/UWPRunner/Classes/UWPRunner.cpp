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


#include <QFile>
#include <QXmlStreamReader>

#include "Concurrency/Thread.h"
#include "Debug/DVAssert.h"
#include "FileSystem/FileSystem.h"
#include "Functional/Function.h"
#include "Network/NetConfig.h"
#include "Network/Services/LogConsumer.h"
#include "Platform/TemplateWin32/UAPNetworkHelper.h"
#include "TeamcityOutput/TeamCityTestsOutput.h"

#include "AppxBundleHelper.h"
#include "ArchiveExtraction.h"
#include "SvcHelper.h"
#include "runner.h"
#include "RegKey.h"
#include "UWPLogConsumer.h"
#include "UWPRunner.h"

using namespace DAVA;

String GetCurrentArchitecture();
QString GetQtWinRTRunnerProfile(const String& profile, const FilePath& manifest);
FilePath ExtractManifest(const FilePath& package);

UWPRunner::UWPRunner(const PackageOptions& opt)
    : options(opt)
    , logConsumer(new UWPLogConsumer)
{
    //install slot to log consumer
    logConsumerConnectionID = logConsumer->newMessageNotifier.Connect([this](const String& logStr)
    {
        NetLogOutput(logStr);
    });

    //if package is bundle, extract concrete package from it
    if (AppxBundleHelper::IsBundle(options.mainPackage))
    {
        ProcessBundlePackage();
    }
    else
    {
        options.packageToInstall = options.mainPackage;
    }

    //Extract manifest from package
    Logger::Info("Extracting manifest...");
    FilePath manifest = ExtractManifest(options.packageToInstall);
    if (manifest.IsEmpty())
    {
        DVASSERT_MSG(false, "Can't extract manifest file from package");
        return;
    }

    //figure out if app should be started on mobile device
    qtProfile = GetQtWinRTRunnerProfile(options.profile, manifest).toStdString();
    FileSystem::Instance()->DeleteFile(manifest);

    //Init network
    if (!options.installOnly)
    {
        Logger::Info("Initializing network...");
        InitializeNetwork(qtProfile == "appxphone");
    }
}

UWPRunner::~UWPRunner()
{
    cleanNeeded.Emit();
    logConsumer->newMessageNotifier.Disconnect(logConsumerConnectionID);

    if (!options.installOnly)
    {
        UnInitializeNetwork();
    }
}

void UWPRunner::Run()
{
    //Create Qt runner
    Logger::Info("Preparing to launch...");

    QStringList resources;
    for (const auto& x : options.resources)
    {
        resources.push_back(QString::fromStdString(x));
    }

    Runner runner(QString::fromStdString(options.mainPackage),
        QString::fromStdString(options.packageToInstall),
        resources,
        QString::fromStdString(options.dependencies),
        QStringList(),
        QString::fromStdString(qtProfile));

    //Check runner state
    if (!runner.isValid())
    {
        DVASSERT_MSG(false, "Runner core is not valid");
        return;
    }

    Run(runner);
}

void UWPRunner::Run(Runner& runner)
{
    //installing and starting application
    if (!options.runOnly)
    {
        Logger::Info("Installing package...");
        if (!runner.install(true))
        {
            DVASSERT_MSG(false, "Can't install application package");
            return;
        }
    }

    if (options.installOnly)
    {
        return;
    }

    Logger::Info("Starting application...");
    if (!runner.start())
    {
        DVASSERT_MSG(false, "Can't install application package");
        return;
    }

    //post-start cleaning
    cleanNeeded.Emit();

    //wait application exit
    WaitApp();
}

void UWPRunner::WaitApp()
{
    Logger::Info("Waiting app exit...");

    while (!logConsumer->IsSessionEnded())
    {
        Net::NetCore::Instance()->Poll();
        Thread::Sleep(10);
    }
}

void UWPRunner::ProcessBundlePackage()
{
    FilePath package = options.mainPackage;
    bundleHelper.reset(new AppxBundleHelper(package));
    cleanNeeded.Connect([this] { bundleHelper.reset(); });

    //try to extract package for specified architecture
    if (!options.architecture.empty())
    {
        package = bundleHelper->GetApplicationForArchitecture(options.architecture);
    }
    //try to extract package for current architecture
    else
    {
        package = bundleHelper->GetApplicationForArchitecture(GetCurrentArchitecture());

        //try to extract package for any architecture
        if (package.IsEmpty())
        {
            Vector<AppxBundleHelper::PackageInfo> applications = bundleHelper->GetApplications();
            package = bundleHelper->GetApplication(applications.at(0).name);
        }
    }

    DVASSERT_MSG(!package.IsEmpty(), "Can't extract app package from bundle");
    if (!package.IsEmpty())
    {
        Vector<AppxBundleHelper::PackageInfo> resources = bundleHelper->GetResources();
        for (const auto& x : resources)
        {
            options.resources.push_back(x.path.GetAbsolutePathname());
        }

        options.packageToInstall = package.GetAbsolutePathname();
    }
}

void UWPRunner::InitializeNetwork(bool isMobileDevice)
{
    using namespace Net;

    if (isMobileDevice)
    {
        bool ipOverUsbConfigured = ConfigureIpOverUsb();
        if (!ipOverUsbConfigured)
        {
            DVASSERT_MSG(false, "Cannot configure IpOverUSB service");
            return;
        }
    }

    std::shared_ptr<UWPLogConsumer> consumer = logConsumer;
    NetCore::Instance()->RegisterService(
        NetCore::SERVICE_LOG,
        [consumer](uint32 serviceId, void*) -> IChannelListener* { return consumer.get(); },
        [] (IChannelListener* obj, void*) -> void {});

    eNetworkRole role;
    Endpoint endPoint;
    if (isMobileDevice)
    {
        role = CLIENT_ROLE;
        endPoint = Endpoint(UAPNetworkHelper::UAP_IP_ADDRESS, UAPNetworkHelper::UAP_MOBILE_TCP_PORT);
    }
    else
    {
        role = SERVER_ROLE;
        endPoint = Endpoint(UAPNetworkHelper::UAP_DESKTOP_TCP_PORT);
    }

    NetConfig config(role);
    config.AddTransport(TRANSPORT_TCP, endPoint);
    config.AddService(NetCore::SERVICE_LOG);

    const uint32 timeout = 5 * 60 * 1000; //5 min
    controllerId = NetCore::Instance()->CreateController(config, nullptr, timeout);
}

void UWPRunner::UnInitializeNetwork()
{
    if (controllerId != DAVA::Net::NetCore::INVALID_TRACK_ID)
    {
        Net::NetCore::Instance()->DestroyControllerBlocked(controllerId);
        controllerId = DAVA::Net::NetCore::INVALID_TRACK_ID;
    }
}

bool UWPRunner::UpdateIpOverUsbConfig(RegKey& key)
{
    const String desiredAddr = UAPNetworkHelper::UAP_IP_ADDRESS;
    const DWORD  desiredPort = UAPNetworkHelper::UAP_MOBILE_TCP_PORT;
    bool changed = false;

    String address = key.QueryString("DestinationAddress");
    if (address != desiredAddr)
    {
        if (!key.SetValue("DestinationAddress", desiredAddr))
        {
            DVASSERT_MSG(false, "Unable to set DestinationAddress");
            return false;
        }
        changed = true;
    }

    DWORD port = key.QueryDWORD("DestinationPort");
    if (port != desiredPort)
    {
        if (!key.SetValue("DestinationPort", desiredPort))
        {
            DVASSERT_MSG(false, "Unable to set DestinationPort");
            return false;
        }
        changed = true;
    }

    address = key.QueryString("LocalAddress");
    if (address != desiredAddr)
    {
        if (!key.SetValue("LocalAddress", desiredAddr))
        {
            DVASSERT_MSG(false, "Unable to set LocalAddress");
            return false;
        }
        changed = true;
    }

    port = key.QueryDWORD("LocalPort");
    if (port != desiredPort)
    {
        if (!key.SetValue("LocalPort", desiredPort))
        {
            DVASSERT_MSG(false, "Unable to set LocalPort");
            return false;
        }
        changed = true;
    }

    return changed;
}

bool UWPRunner::RestartIpOverUsb()
{
    //open service
    SvcHelper service("IpOverUsbSvc");
    if (!service.IsInstalled())
    {
        DVASSERT_MSG(false, "Can't open IpOverUsb service");
        return false;
    }

    //stop it
    if (!service.Stop())
    {
        DVASSERT_MSG(false, "Can't stop IpOverUsb service");
        return false;
    }

    //start it
    if (!service.Start())
    {
        DVASSERT_MSG(false, "Can't start IpOverUsb service");
        return false;
    }

    //waiting for service starting
    Thread::Sleep(1000);

    return true;
}

bool UWPRunner::ConfigureIpOverUsb()
{
    bool needRestart = false;

    //open or create key
    RegKey key(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\IpOverUsbSdk\\DavaDebugging", true);
    if (!key.IsExist())
    {
        DVASSERT_MSG(false, "Can't open or create key");
        return false;
    }
    needRestart |= key.IsCreated();

    //update config values
    bool result = UpdateIpOverUsbConfig(key);
    /*if (!result.IsSet())
    {
    DVASSERT_MSG(false, "Unable to update IpOverUsb service config");
    return false;
    }*/
    needRestart |= result;

    //restart service to applying new config
    if (needRestart)
        return RestartIpOverUsb();
    return true;
}

void UWPRunner::NetLogOutput(const String& logString)
{
    //incoming string is formatted in style "[ip:port] date time message"
    //extract only message text
    String logLevel;
    String message;

    size_t spaces = 0;
    for (auto i : logString)
    {
        if (::isspace(i))
        {
            spaces++;
        }

        if (spaces == 3)
        {
            logLevel += i;
        }
        else if (spaces >= 4)
        {
            message += i;
        }
    }

    if (logLevel.empty())
    {
        return;
    }

    //remove first space
    logLevel = logLevel.substr(1);
    message = message.substr(1);

    if (options.useTeamCityTestOutput)
    {
        Logger* logger = Logger::Instance();
        Logger::eLogLevel ll = logger->GetLogLevelFromString(logLevel.c_str());

        if (ll != Logger::LEVEL__DISABLE)
        {
            TeamcityTestsOutput testOutput;
            testOutput.Output(ll, message.c_str());
        }
    }
    else
    {
        printf("[%s] %s", logLevel.c_str(), message.c_str());
        if (message.back() != '\n' || message.back() != '\r')
        {
            printf("\n");
        }
    }
}

String GetCurrentArchitecture()
{
    return sizeof(void*) == 4 ? "x86" : "x64";
}

QString GetQtWinRTRunnerProfile(const String& profile, const FilePath& manifest)
{
    //if profile is set, just convert it
    if (!profile.empty())
    {
        return profile == "local" ? QStringLiteral("appx") : QStringLiteral("appxphone");
    }

    //else try to find out profile from manifest
    QFile file(QString::fromStdString(manifest.GetAbsolutePathname()));
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    QXmlStreamReader xml(&file);

    while (!xml.atEnd() && !xml.hasError())
    {
        QXmlStreamReader::TokenType token = xml.readNext();
        if (token != QXmlStreamReader::StartElement ||
            xml.name() != QStringLiteral("Identity"))
        {
            continue;
        }

        QXmlStreamAttributes attributes = xml.attributes();
        for (const auto& attribute : attributes)
        {
            if (attribute.name() == QStringLiteral("ProcessorArchitecture"))
            {
                QString arch = attribute.value().toString().toLower();
                if (arch == QStringLiteral("arm"))
                {
                    return QStringLiteral("appxphone");
                }
                else
                {
                    return QStringLiteral("appx");
                }
            }
        }
    }

    return "";
}

FilePath ExtractManifest(const FilePath& package)
{
    String manifestFilePath = package.GetAbsolutePathname() + "_manifest.xml";

    //extract manifest from appx
    if (ExtractFileFromArchive(package.GetAbsolutePathname(),
                               "AppxManifest.xml",
                               manifestFilePath))
    {
        return manifestFilePath;
    }
    return FilePath();
}