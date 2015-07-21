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

#include "Platform/TemplateWin32/Dispatcher.h"

#if defined(__DAVAENGINE_WIN_UAP__)

#include "Debug/DVAssert.h"
#include "Concurrency/LockGuard.h"
#include "Concurrency/UniqueLock.h"
#include "Concurrency/LockGuard.h"

#include "Platform/TemplateWin32/CorePlatformWinUAP.h"

namespace DAVA
{

namespace
{

// Wrapper to prepare task for running in Dispatcher's thread context and waiting task completion
class TaskWrapper final
{
public:
    TaskWrapper(std::function<void()>&& task_)
        : task(std::move(task_))
    {}

    void RunTask()
    {
        task();
        {
            LockGuard<Mutex> lock(mutex);
            taskDone = true;
        }
        cv.NotifyOne();
    }

    void WaitTaskComplete()
    {
        UniqueLock<Mutex> lock(mutex);
        while (!taskDone)
            cv.Wait(lock);
    }

private:
    Mutex mutex;
    ConditionVariable cv;
    std::function<void()> task;
    bool taskDone = false;
};

}   // unnamed namespace

Dispatcher::BlockingTaskWrapper::BlockingTaskWrapper(BlockingTaskWrapper&& other)
    : dispatcher(std::move(other.dispatcher))
    , task(std::move(other.task))
    , taskDone(std::move(taskDone))
{
    other.dispatcher = nullptr;
    other.taskDone = false;
}

void Dispatcher::BlockingTaskWrapper::RunTask()
{
    task();
    {
        LockGuard<Mutex> lock(dispatcher->mutex);
        taskDone = true;
    }
    dispatcher->cv.NotifyAll();
}

void Dispatcher::BlockingTaskWrapper::WaitTaskComplete()
{
    DVASSERT(dispatcher->boundThreadId == Thread::GetCurrentId());

    UniqueLock<Mutex> lock(dispatcher->mutex);
    while (!taskDone)
    {
        if (!dispatcher->taskQueue.empty())
        {
            lock.Unlock();
            dispatcher->ProcessTasks();
            lock.Lock();
        }
        dispatcher->cv.Wait(lock);
    }
}

Dispatcher::Dispatcher()
    : boundThreadId(Thread::GetCurrentId())
{}

void Dispatcher::BindToCurrentThread()
{
    boundThreadId = Thread::GetCurrentId();
}

bool Dispatcher::InBlockingCall() const
{
    if (blockingCall.test_and_set() == false)
    {
        blockingCall.clear();
        return false;
    }
    return true;
}

void Dispatcher::ProcessTasks()
{
    DVASSERT(boundThreadId == Thread::GetCurrentId());

    std::list<std::function<void()>> tasksToExecute;
    {
        LockGuard<Mutex> guard(mutex);
        tasksToExecute.swap(taskQueue);
    }

    for (std::function<void()>& task : tasksToExecute)
    {
        task();
    }
}

void Dispatcher::ScheduleTask(std::function<void()>&& task)
{
    {
        LockGuard<Mutex> guard(mutex);
        taskQueue.emplace_back(std::move(task));
    }
    cv.NotifyAll();
}

void Dispatcher::ScheduleTaskAndWait(std::function<void()>&& task)
{
    // TODO: maybe call task as subroutine without scheduling
    DVASSERT(boundThreadId != Thread::GetCurrentId());
    DVASSERT(InBlockingCall() == false);

    blockingCall.test_and_set();
    TaskWrapper taskWrapper(std::move(task));
    ScheduleTask(std::function<void()>([&taskWrapper]() { taskWrapper.RunTask(); }));
    taskWrapper.WaitTaskComplete();
    blockingCall.clear();
}

}   // namespace DAVA

#endif  // __DAVAENGINE_WIN_UAP__
