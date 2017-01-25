#ifndef __FRAMEWORK__DEVICEINFO_IOS__
#define __FRAMEWORK__DEVICEINFO_IOS__

#include "Base/Platform.h"

#if defined(__DAVAENGINE_IPHONE__)

#include "Platform/DeviceInfoPrivateBase.h"

namespace DAVA
{
class DeviceInfoPrivate : public DeviceInfoPrivateBase
{
public:
    DeviceInfoPrivate();
    ~DeviceInfoPrivate();
    DeviceInfo::ePlatform GetPlatform();
    String GetPlatformString();
    String GetVersion();
    String GetManufacturer();
    String GetModel();
    String GetLocale();
    String GetRegion();
    String GetTimeZone();
    String GetUDID();
    WideString GetName();
    String GetHTTPProxyHost();
    String GetHTTPNonProxyHosts();
    int32 GetHTTPProxyPort();
    int32 GetZBufferSize();
    eGPUFamily GetGPUFamilyImpl() override;
    DeviceInfo::NetworkInfo GetNetworkInfo();
    List<DeviceInfo::StorageInfo> GetStoragesList();
    bool IsHIDConnected(DeviceInfo::eHIDType type);
    bool IsTouchPresented();
    String GetCarrierName();

#if !defined(__DAVAENGINE_COREV2__)
    DeviceInfo::ScreenInfo screenInfo;
    DeviceInfo::ScreenInfo& GetScreenInfo();
    void InitializeScreenInfo();
#endif

private:
    // Use void* to avoid mentioning obj-c classes in this header
    void* telephonyNetworkInfo = nullptr;
    void* lastCarrierName = nullptr;
};

}; // namespace DAVA

#endif //defined(__DAVAENGINE_IPHONE__)

#endif /* defined(__FRAMEWORK__DEVICEINFO_IOS__) */
