#include "Notification/Private/NativeDelegateNotImplemented.h"

#if defined(__DAVAENGINE_WIN32__) || defined(__DAVAENGINE_QT__)
#if defined(__DAVAENGINE_COREV2__)

namespace DAVA
{
namespace Private
{
NativeDelegate::NativeDelegate(LocalNotificationController& controller)
{
}
} // namespace Private
} // namespace DAVA
#endif // __DAVAENGINE_COREV2__
#endif // defined(__DAVAENGINE_WIN32__) || defined(__DAVAENGINE_QT__)
