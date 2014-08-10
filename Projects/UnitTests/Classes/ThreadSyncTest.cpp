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



#include "ThreadSyncTest.h"

ThreadSyncTest::ThreadSyncTest()
: TestTemplate<ThreadSyncTest>("ThreadSyncTest")
{
    RegisterFunction(this, &ThreadSyncTest::ThreadSyncTestFunction, "ThreadSyncTestFunction", NULL);
    RegisterFunction(this, &ThreadSyncTest::ThreadSleepTestFunction, "ThreadSleepTestFunction", NULL);
    RegisterFunction(this, &ThreadSyncTest::TestThread, "TestThread", NULL);
}

void ThreadSyncTest::LoadResources()
{
    someThread = Thread::Create(Message(this, &ThreadSyncTest::SomeThreadFunc));
}

void ThreadSyncTest::UnloadResources()
{
    SafeRelease(someThread);
}

void ThreadSyncTest::SomeThreadFunc(BaseObject * caller, void * callerData, void * userData)
{
    someValue = 0;
    Thread::Signal(&cv);
}

void ThreadSyncTest::ThreadSleepTestFunction(PerfFuncData * data)
{
    uint64 time = SystemTimer::Instance()->AbsoluteMS();
    Thread::SleepThread(300);
    uint64 elapsedTime = SystemTimer::Instance()->AbsoluteMS() - time;
    TEST_VERIFY(elapsedTime >= 300);
}

void ThreadSyncTest::ThreadSyncTestFunction(PerfFuncData * data)
{
    someValue = -1;
    someThread->Start();
    Thread::Wait(&cv);
    TEST_VERIFY(someValue == 0);
}

void ThreadSyncTest::InfiniteThreadFunction(BaseObject * caller, void * callerData, void * userData)
{
    Thread *thread = static_cast<Thread *>(caller);
    while(thread && Thread::STATE_RUNNING == thread->GetState())
    {
        Thread::SleepThread(200);
    }
    
}

void ThreadSyncTest::ShortThreadFunction(BaseObject * caller, void * callerData, void * userData)
{
    uint32 i = 100;
    Thread *thread = static_cast<Thread *>(caller);
    while(thread && i-- > 0)
    {
        Thread::SleepThread(1);
        if (Thread::STATE_CANCELLING == thread->GetState())
            break;
    }
    
}

void ThreadSyncTest::TestThread(PerfFuncData * data)
{
    Thread *infiniteThread = Thread::Create(Message(this, &ThreadSyncTest::InfiniteThreadFunction));
    
    TEST_VERIFY(Thread::STATE_CREATED == infiniteThread->GetState());
    
    infiniteThread->Start();
    
    uint32 timeout = 3000;
    while (timeout-- > 0 && Thread::STATE_RUNNING != infiniteThread->GetState())
    {
        Thread::SleepThread(1);
    }
    TEST_VERIFY(timeout > 0);

    infiniteThread->Cancel();
    infiniteThread->Join();
    TEST_VERIFY(Thread::STATE_CANCELLED == infiniteThread->GetState());

    Thread *shortThread = Thread::Create(Message(this, &ThreadSyncTest::ShortThreadFunction));
    shortThread->Start();
    shortThread->Join();
    TEST_VERIFY(Thread::STATE_ENDED == shortThread->GetState());
    
    infiniteThread->Start();
    shortThread->Start();

    Thread::SleepThread(50);
    Thread::KillAll();
    
    shortThread->Join();
    TEST_VERIFY(Thread::STATE_KILLED == shortThread->GetState());
    
    infiniteThread->Join();
    TEST_VERIFY(Thread::STATE_KILLED == infiniteThread->GetState());
    
    
    SafeRelease(infiniteThread);
    SafeRelease(shortThread);
}



