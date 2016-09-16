#include "Tests/DeviceInfoTest.h"

#include "Infrastructure/GameCore.h"

using namespace DAVA;

DeviceInfoTest::DeviceInfoTest(GameCore* g)
    : BaseScreen(g, "DeviceInfoTest")
{
    DeviceInfo::GetHIDConnectionSignal(DAVA::DeviceInfo::eHIDType::HID_POINTER_TYPE).Connect(this, &DeviceInfoTest::OnInputChanged);
    DeviceInfo::GetHIDConnectionSignal(DAVA::DeviceInfo::eHIDType::HID_MOUSE_TYPE).Connect(this, &DeviceInfoTest::OnInputChanged);
    DeviceInfo::GetHIDConnectionSignal(DAVA::DeviceInfo::eHIDType::HID_JOYSTICK_TYPE).Connect(this, &DeviceInfoTest::OnInputChanged);
    DeviceInfo::GetHIDConnectionSignal(DAVA::DeviceInfo::eHIDType::HID_GAMEPAD_TYPE).Connect(this, &DeviceInfoTest::OnInputChanged);
    DeviceInfo::GetHIDConnectionSignal(DAVA::DeviceInfo::eHIDType::HID_KEYBOARD_TYPE).Connect(this, &DeviceInfoTest::OnInputChanged);
    DeviceInfo::GetHIDConnectionSignal(DAVA::DeviceInfo::eHIDType::HID_KEYPAD_TYPE).Connect(this, &DeviceInfoTest::OnInputChanged);
    DeviceInfo::GetHIDConnectionSignal(DAVA::DeviceInfo::eHIDType::HID_SYSTEM_CONTROL_TYPE).Connect(this, &DeviceInfoTest::OnInputChanged);
    DeviceInfo::GetHIDConnectionSignal(DAVA::DeviceInfo::eHIDType::HID_TOUCH_TYPE).Connect(this, &DeviceInfoTest::OnInputChanged);

    DeviceInfo::carrierNameChanged.Connect(this, &DeviceInfoTest::OnCarrierChanged);
}

void DeviceInfoTest::LoadResources()
{
    Font* font = FTFont::Create("~res:/Fonts/korinna.ttf");
    DVASSERT(font);

    font->SetSize(24.0f);
    Size2i screenSize = VirtualCoordinatesSystem::Instance()->GetVirtualScreenSize();
    BaseScreen::LoadResources();
    info = new UIStaticText(Rect(0.f, 0.f, static_cast<float32>(screenSize.dx), static_cast<float32>(screenSize.dy)));
    info->SetTextColor(Color::White);
    info->SetFont(font);
    info->SetMultiline(true);
    info->SetTextAlign(ALIGN_LEFT | ALIGN_TOP);
    AddControl(info);
    UpdateTestInfo();

    //TODO: Initialize resources here
}

void DeviceInfoTest::UpdateTestInfo()
{
    if (nullptr == info)
    {
        return;
    }
    std::wstringstream infoStream;

    infoStream << L"DeviceInfo\n";
    infoStream << L"GetPlatform() :" << int(DeviceInfo::GetPlatform()) << L"\n";
    infoStream << L"GetPlatformString() :" << StringToWString(DeviceInfo::GetPlatformString()) << L"\n";
    infoStream << L"GetVersion() :" << StringToWString(DeviceInfo::GetVersion()) << L"\n";
    infoStream << L"GetManufacturer() :" << StringToWString(DeviceInfo::GetManufacturer()) << L"\n";
    infoStream << L"GetModel() :" << StringToWString(DeviceInfo::GetModel()) << L"\n";
    infoStream << L"GetLocale() :" << StringToWString(DeviceInfo::GetLocale()) << L"\n";
    infoStream << L"GetRegion() :" << StringToWString(DeviceInfo::GetRegion()) << L"\n";
    infoStream << L"GetTimeZone() :" << StringToWString(DeviceInfo::GetTimeZone()) << L"\n";
    infoStream << L"GetUDID() :" << StringToWString(DeviceInfo::GetUDID()) << L"\n";
    infoStream << L"GetName() :" << DeviceInfo::GetName() << L"\n";
    infoStream << L"GetHTTPProxyHost() :" << StringToWString(DeviceInfo::GetHTTPProxyHost()) << L"\n";
    infoStream << L"GetHTTPNonProxyHosts() :" << StringToWString(DeviceInfo::GetHTTPNonProxyHosts()) << L"\n";
    infoStream << L"GetHTTPProxyPort() :" << DeviceInfo::GetHTTPProxyPort() << L"\n";
    infoStream << L"GetScreenInfo() :" << L"width: " << DeviceInfo::GetScreenInfo().width << L", height: " << DeviceInfo::GetScreenInfo().height << L", scale: " << DeviceInfo::GetScreenInfo().scale << L"\n";
    infoStream << L"GetZBufferSize() :" << DeviceInfo::GetZBufferSize() << L"\n";
    infoStream << L"GetGPUFamily() :" << int(DeviceInfo::GetGPUFamily()) << L"\n";
    infoStream << L"GetNetworkInfo() :" << L"networkType: " << int(DeviceInfo::GetNetworkInfo().networkType) << L", signalStrength: " << int(DeviceInfo::GetNetworkInfo().signalStrength) << L"\n";
    infoStream << L"GetStoragesList() :" << L"Size: " << (DeviceInfo::GetStoragesList().size()) << L"\n";
    infoStream << L"GetCpuCount() :" << (DeviceInfo::GetCpuCount()) << L"\n";
    infoStream << L"GetCarrierName() :" << StringToWString(DeviceInfo::GetCarrierName()) << L"\n";

    infoStream << L"HIDDevices() :";
    infoStream << L"pointer(" << ((hidDevices[DeviceInfo::HID_POINTER_TYPE] = DeviceInfo::IsHIDConnected(DeviceInfo::HID_POINTER_TYPE)) ? "y" : "n") << "), ";
    infoStream << L"mouse(" << ((hidDevices[DeviceInfo::HID_MOUSE_TYPE] = DeviceInfo::IsHIDConnected(DeviceInfo::HID_MOUSE_TYPE)) ? "y" : "n") << "), ";
    infoStream << L"joystick(" << ((hidDevices[DeviceInfo::HID_JOYSTICK_TYPE] = DeviceInfo::IsHIDConnected(DeviceInfo::HID_JOYSTICK_TYPE)) ? "y" : "n") << "), ";
    infoStream << L"gamepad(" << ((hidDevices[DeviceInfo::HID_GAMEPAD_TYPE] = DeviceInfo::IsHIDConnected(DeviceInfo::HID_GAMEPAD_TYPE)) ? "y" : "n") << "), ";
    infoStream << L"keyboard(" << ((hidDevices[DeviceInfo::HID_KEYBOARD_TYPE] = DeviceInfo::IsHIDConnected(DeviceInfo::HID_KEYBOARD_TYPE)) ? "y" : "n") << "), ";
    infoStream << L"keypad(" << ((hidDevices[DeviceInfo::HID_KEYPAD_TYPE] = DeviceInfo::IsHIDConnected(DeviceInfo::HID_KEYPAD_TYPE)) ? "y" : "n") << "), ";
    infoStream << L"system(" << ((hidDevices[DeviceInfo::HID_SYSTEM_CONTROL_TYPE] = DeviceInfo::IsHIDConnected(DeviceInfo::HID_SYSTEM_CONTROL_TYPE)) ? "y" : "n") << "), ";
    infoStream << L"touch(" << ((hidDevices[DeviceInfo::HID_TOUCH_TYPE] = DeviceInfo::IsHIDConnected(DeviceInfo::HID_TOUCH_TYPE)) ? "y" : "n") << ")";
    infoStream << L"\n";

    info->SetText(infoStream.str());
}

void DeviceInfoTest::OnInputChanged(DAVA::DeviceInfo::eHIDType hidType, bool connected)
{
    hidDevices[hidType] = connected;
    UpdateTestInfo();
}

void DeviceInfoTest::OnCarrierChanged(const DAVA::String&)
{
    UpdateTestInfo();
}

void DeviceInfoTest::UnloadResources()
{
    SafeRelease(info);

    BaseScreen::UnloadResources();
    //TODO: Release resources here
}
