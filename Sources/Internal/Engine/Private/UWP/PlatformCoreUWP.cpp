#if defined(__DAVAENGINE_COREV2__)

#include "Engine/Private/UWP/PlatformCoreUWP.h"

#if defined(__DAVAENGINE_WIN_UAP__)

#include "Engine/Window.h"
#include "Engine/Private/EngineBackend.h"
#include "Engine/Private/Dispatcher/MainDispatcherEvent.h"
#include "Engine/Private/UWP/Window/WindowBackendUWP.h"

#include "Platform/SystemTimer.h"
#include "Concurrency/Thread.h"
#include "Logger/Logger.h"
#include "Utils/Utils.h"
#include "Platform/DeviceInfo.h"

extern int DAVAMain(DAVA::Vector<DAVA::String> cmdline);

namespace DAVA
{
namespace Private
{
bool PlatformCore::isPhoneContractPresent = false;

PlatformCore::PlatformCore(EngineBackend* engineBackend_)
    : engineBackend(engineBackend_)
    , dispatcher(engineBackend->GetDispatcher())
{
    using ::Windows::Foundation::Metadata::ApiInformation;
    isPhoneContractPresent = ApiInformation::IsApiContractPresent("Windows.Phone.PhoneContract", 1);
}

PlatformCore::~PlatformCore() = default;

void PlatformCore::Init()
{
}

void PlatformCore::Run()
{
    engineBackend->OnGameLoopStarted();

    while (!quitGameThread)
    {
        uint64 frameBeginTime = SystemTimer::Instance()->AbsoluteMS();

        int32 fps = engineBackend->OnFrame();

        uint64 frameEndTime = SystemTimer::Instance()->AbsoluteMS();
        uint32 frameDuration = static_cast<uint32>(frameEndTime - frameBeginTime);

        int32 sleep = 1;
        if (fps > 0)
        {
            sleep = 1000 / fps - frameDuration;
            if (sleep < 1)
                sleep = 1;
        }
        Sleep(sleep);
    }

    engineBackend->OnGameLoopStopped();
    engineBackend->OnEngineCleanup();
}

void PlatformCore::PrepareToQuit()
{
    engineBackend->PostAppTerminate(true);
}

void PlatformCore::Quit()
{
    quitGameThread = true;
}

void PlatformCore::OnLaunched()
{
    if (!gameThreadRunning)
    {
        Thread* gameThread = Thread::Create(MakeFunction(this, &PlatformCore::GameThread));
        gameThread->Start();
        gameThread->BindToProcessor(0);
        // TODO: make Thread detachable
        //gameThread->Detach();
        //gameThread->Release();

        gameThreadRunning = true;
    }
}

void PlatformCore::OnActivated()
{
}

void PlatformCore::OnWindowCreated(::Windows::UI::Xaml::Window ^ xamlWindow)
{
    // TODO: think about binding XAML window to prior created Window instance
    Window* primaryWindow = engineBackend->GetPrimaryWindow();
    if (primaryWindow == nullptr)
    {
        primaryWindow = engineBackend->InitializePrimaryWindow();
    }
    WindowBackend* windowBackend = primaryWindow->GetBackend();
    windowBackend->BindXamlWindow(xamlWindow);
}

void PlatformCore::OnSuspending()
{
    dispatcher->SendEvent(MainDispatcherEvent(MainDispatcherEvent::APP_SUSPENDED)); // Blocking call !!!
}

void PlatformCore::OnResuming()
{
    dispatcher->PostEvent(MainDispatcherEvent(MainDispatcherEvent::APP_RESUMED));
}

void PlatformCore::OnUnhandledException(::Windows::UI::Xaml::UnhandledExceptionEventArgs ^ arg)
{
    Logger::Error("Unhandled exception: hresult=0x%08X, message=%s", arg->Exception, WStringToString(arg->Message->Data()).c_str());
}

void PlatformCore::OnBackPressed()
{
    dispatcher->PostEvent(MainDispatcherEvent(MainDispatcherEvent::BACK_NAVIGATION));
}

void PlatformCore::OnGamepadAdded(::Windows::Gaming::Input::Gamepad ^ /*gamepad*/)
{
    dispatcher->PostEvent(MainDispatcherEvent::CreateGamepadAddedEvent(0));
}

void PlatformCore::OnGamepadRemoved(::Windows::Gaming::Input::Gamepad ^ /*gamepad*/)
{
    dispatcher->PostEvent(MainDispatcherEvent::CreateGamepadRemovedEvent(0));
}

void PlatformCore::OnDpiChanged()
{
    engineBackend->UpdateDisplayConfig();
}

void PlatformCore::GameThread()
{
    Vector<String> cmdline = engineBackend->GetCommandLine();
    DAVAMain(std::move(cmdline));

    using namespace ::Windows::UI::Xaml;
    Application::Current->Exit();
}

} // namespace Private
} // namespace DAVA

#endif // __DAVAENGINE_WIN_UAP__
#endif // __DAVAENGINE_COREV2__
