#include "DAVAEngine.h"
#include "GameCore.h"
#include "Platform/DeviceInfo.h"
#include "Base/Platform.h"

using namespace DAVA;

const int32 WIDTH = 1024;
const int32 HEIGHT = 768;

void FrameworkDidLaunched()
{
#if defined(__DAVAENGINE_COREV2__)
#else
    int32 screenWidth = 0;
    int32 screenHeight = 0;

    KeyedArchive* appOptions = new KeyedArchive();

    appOptions->SetString(String("title"), String("TestBed"));

#if defined(__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_ANDROID__)
    screenWidth = Max(DeviceInfo::GetScreenInfo().width, DeviceInfo::GetScreenInfo().height);
    screenHeight = Min(DeviceInfo::GetScreenInfo().width, DeviceInfo::GetScreenInfo().height);

    appOptions->SetInt32("orientation", Core::SCREEN_ORIENTATION_LANDSCAPE_LEFT);
    appOptions->SetInt32("renderer", rhi::RHI_GLES2);
    appOptions->SetBool("iPhone_autodetectScreenScaleFactor", true);

    DAVA::UIControlSystem::Instance()->vcs->SetProportionsIsFixed(false);

#elif defined(__DAVAENGINE_MACOS__)
    screenWidth = WIDTH;
    screenHeight = HEIGHT;

    appOptions->SetInt32("renderer", rhi::RHI_GLES2);
    appOptions->SetInt32("fullscreen", 0);
    appOptions->SetInt32("bpp", 32);

#elif defined(__DAVAENGINE_WIN_UAP__)
    screenWidth = DeviceInfo::GetScreenInfo().width;
    screenHeight = DeviceInfo::GetScreenInfo().height;

    appOptions->SetInt32("renderer", rhi::RHI_DX11);
    appOptions->SetInt32("fullscreen", 0);
    appOptions->SetInt32("bpp", 32);

    DAVA::UIControlSystem::Instance()->vcs->SetProportionsIsFixed(false);
#elif defined(__DAVAENGINE_WIN32__)
    screenWidth = WIDTH;
    screenHeight = HEIGHT;

    appOptions->SetInt32("renderer", rhi::RHI_DX9); // TODO on DX9 - assert
    appOptions->SetInt32("fullscreen", 0);
    appOptions->SetInt32("bpp", 32);

#else
    screenWidth = WIDTH;
    screenHeight = HEIGHT;

    appOptions->SetInt32("renderer", rhi::RHI_DX9);
    appOptions->SetInt32("fullscreen", 0);
    appOptions->SetInt32("bpp", 32);

#endif

    // For testing purpose limit minimum window size to 960x640
    appOptions->SetInt32("min-width", 960);
    appOptions->SetInt32("min-height", 640);

    appOptions->SetInt32("rhi_threaded_frame_count", 2);
    appOptions->SetInt32("width", screenWidth);
    appOptions->SetInt32("height", screenHeight);

    DAVA::UIControlSystem::Instance()->vcs->SetVirtualScreenSize(WIDTH, HEIGHT);
    DAVA::UIControlSystem::Instance()->vcs->RegisterAvailableResourceSize(screenWidth, screenHeight, "Gfx");

    GameCore* core = new GameCore();
    DAVA::Core::SetApplicationCore(core);
    DAVA::Core::Instance()->SetOptions(appOptions);
#endif //__DAVAENGINE_COREV2__
}

void FrameworkWillTerminate()
{
}
