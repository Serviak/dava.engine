// Utility header with forward declarations of Engine-related public and private classes
namespace DAVA
{
class Engine;
class Window;
class EngineContext;
class NativeService;

template <typename T>
class Dispatcher;

namespace Private
{
struct MainDispatcherEvent;
struct UIDispatcherEvent;

using MainDispatcher = Dispatcher<MainDispatcherEvent>;
using UIDispatcher = Dispatcher<UIDispatcherEvent>;

class EngineBackend;
class PlatformCore;
class WindowBackend;

#if defined(__DAVAENGINE_QT__)

#elif defined(__DAVAENGINE_WIN32__)

#elif defined(__DAVAENGINE_WIN_UAP__)
ref struct WindowNativeBridge;
#elif defined(__DAVAENGINE_MACOS__)
struct CoreNativeBridge;
struct WindowNativeBridge;
#elif defined(__DAVAENGINE_IPHONE__)
struct CoreNativeBridge;
struct WindowNativeBridge;
#elif defined(__DAVAENGINE_ANDROID__)
struct AndroidBridge;
#else
#if defined(__DAVAENGINE_COREV2__)
// Do not emit error when building with old core implementation 
#error "Platform is not implemented"
#endif
#endif

} // namespace Private
} // namespace DAVA
