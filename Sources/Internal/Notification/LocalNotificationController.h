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


#ifndef __DAVAENGINE_LOCAL_NOTIFICATION_CONTROLLER_H__
#define __DAVAENGINE_LOCAL_NOTIFICATION_CONTROLLER_H__

#include "Base/BaseTypes.h"
#include "Base/Singleton.h"
#include "Base/Message.h"
#include "Concurrency/Mutex.h"
#include "Notification/LocalNotificationAndroid.h"
#include "Notification/LocalNotificationNotImplemented.h"


namespace DAVA
{

class LocalNotification;
class LocalNotificationText;
class LocalNotificationProgress;

class LocalNotificationController : public Singleton<LocalNotificationController>
{
	friend class LocalNotification;
public:
    virtual ~LocalNotificationController();
    LocalNotificationProgress *const CreateNotificationProgress(const WideString &title = L"", const WideString &text = L"", const uint32 max = 0, const uint32 current = 0, const bool useSound = false);
    LocalNotificationText *const CreateNotificationText(const WideString &title = L"", const WideString &text = L"", const bool useSound = false);
    void PostDelayedNotification(const WideString &title, const WideString text, int delaySeconds, const bool useSound = false);
    void RemoveAllDelayedNotifications();
    bool Remove(LocalNotification *const notification);
    bool RemoveById(const String &notificationId);
    void Clear();
    void Update();

    LocalNotification *const GetNotificationById(const String &id);
    void OnNotificationPressed(const String &id);

private:
	Mutex notificationsListMutex;
	List<LocalNotification *> notificationsList;
};

}

#endif // __NOTIFICATION_H__
