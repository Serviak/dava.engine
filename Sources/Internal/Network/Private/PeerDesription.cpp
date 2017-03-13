#include <Debug/DVAssert.h>
#include <Utils/UTF8Utils.h>

#include <Network/PeerDesription.h>

namespace DAVA
{
namespace Net
{
PeerDescription::PeerDescription()
    : platformType()
    , gpuFamily()
{
}

PeerDescription::PeerDescription(const String& appName, const NetConfig& config)
    : platformType(DeviceInfo::GetPlatform())
    , platform(DeviceInfo::GetPlatformString())
    , version(DeviceInfo::GetVersion())
    , manufacturer(DeviceInfo::GetManufacturer())
    , model(DeviceInfo::GetModel())
    , udid(DeviceInfo::GetUDID())
    , deviceName(UTF8Utils::EncodeToUTF8(DeviceInfo::GetName()))
    , appName(appName)
#if !defined(__DAVAENGINE_COREV2__)
    , screenInfo(DeviceInfo::GetScreenInfo())
#endif
    , gpuFamily(DeviceInfo::GetGPUFamily())
    , netConfig(config)
{
    DVASSERT(true == netConfig.Validate());
}

PeerDescription& PeerDescription::operator=(PeerDescription&& other)
{
    platformType = std::move(other.platformType);
    platform = std::move(other.platform);
    version = std::move(other.version);
    manufacturer = std::move(other.manufacturer);
    model = std::move(other.model);
    udid = std::move(other.udid);
    deviceName = std::move(other.deviceName);
    appName = std::move(other.appName);
#if !defined(__DAVAENGINE_COREV2__)
    screenInfo = std::move(other.screenInfo);
#endif
    gpuFamily = std::move(other.gpuFamily);

    netConfig = std::move(other.netConfig);
    ifaddr = std::move(other.ifaddr);

    return *this;
}

void PeerDescription::SetNetworkInterfaces(const Vector<IfAddress>& availIfAddr)
{
    ifaddr = availIfAddr;
}

#ifdef __DAVAENGINE_DEBUG__
void PeerDescription::DumpToStdout() const
{
#if !defined(__DAVAENGINE_COREV2__)
    printf("PeerDescription: %s, screen %dx%d, UDID=%s\n", name.c_str(), screenInfo.width, screenInfo.height, udid.c_str());
#else
    printf("PeerDescription: %s-%s, UDID=%s\n", deviceName.c_str(), appName.c_str(), udid.c_str());
#endif
    printf("  %s %s %s %s\n", manufacturer.c_str(), model.c_str(), platform.c_str(), version.c_str());
    printf("  Network interfaces:\n");
    for (size_t i = 0, n = ifaddr.size(); i < n; ++i)
    {
        printf("    %s\n", ifaddr[i].Address().ToString().c_str());
    }
    printf("  Network configuration:\n");
    for (size_t i = 0, n = netConfig.Transports().size(); i < n; ++i)
    {
        const char* s = "unknown";
        switch (netConfig.Transports()[i].type)
        {
        case TRANSPORT_TCP:
            s = "TCP";
            break;
        }
        printf("    %s: %hu\n", s, netConfig.Transports()[i].endpoint.Port());
    }
    printf("    services: ");
    for (size_t i = 0, n = netConfig.Services().size(); i < n; ++i)
    {
        printf("%u; ", netConfig.Services()[i]);
    }
    printf("\n");
}
#endif

} // namespace Net
} // namespace DAVA
