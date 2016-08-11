#include "RenderLoop.h"
#include "rhi_Pool.h"
#include "CommonImpl.h"
#include "FrameLoop.h"
#include "../rhi_Type.h"
#include "Concurrency/AutoResetEvent.h"
#include "Concurrency/Concurrency.h"
#include "Debug/Profiler.h"
#include "Logger/Logger.h"
#include <atomic>

namespace rhi
{
namespace RenderLoop
{
static DAVA::AutoResetEvent framePreparedEvent(false, 400);
static DAVA::AutoResetEvent frameDoneEvent(false, 400);
static DAVA::Thread* renderThread = nullptr;
static uint32 renderThreadFrameCount = 0;
static DAVA::Semaphore renderThredStartedSync(1);

static DAVA::Semaphore renderThreadSuspendSync;
static std::atomic<bool> renderThreadSuspendSyncReached(false);
static std::atomic<bool> renderThreadSuspended(false);

static std::atomic<bool> renderThreadExitPending(false);

static std::atomic<CommonImpl::ImmediateCommand*> pendingImmediateCmd(nullptr);
static DAVA::Mutex pendingImmediateCmdSync;

using DAVA::Logger;

void Present(Handle syncHandle) // called from main thread
{
    TRACE_BEGIN_EVENT((uint32)DAVA::Thread::GetCurrentId(), "", "rhi::present");

    bool res = FrameLoop::FinishFrame(syncHandle);
    if (!res) //if present was called without actual work - need to do nothing here (or should we swap buffers in any case?)
        return;

    uint32 frame_cnt = 0;
    if (renderThreadFrameCount == 0) //single thread render
    {
        FrameLoop::ProcessFrame();
    }
    else //wait for render thread if needed
    {
        if (!renderThreadSuspended)
            framePreparedEvent.Signal();

        TRACE_BEGIN_EVENT((uint32)DAVA::Thread::GetCurrentId(), "", "core_wait_renderer");
        do
        {
            frame_cnt = FrameLoop::FramesCount();
            if (frame_cnt >= renderThreadFrameCount)
                frameDoneEvent.Wait();

        } while (frame_cnt >= renderThreadFrameCount);

        TRACE_END_EVENT((uint32)DAVA::Thread::GetCurrentId(), "", "core_wait_renderer");
    }

    TRACE_END_EVENT((uint32)DAVA::Thread::GetCurrentId(), "", "rhi::present");
}

//------------------------------------------------------------------------------

static void RenderFunc(DAVA::BaseObject* obj, void*, void*)
{
    DispatchPlatform::InitContext();

    renderThredStartedSync.Post();
    Logger::Info("[RHI] render-thread started");

    while (!renderThreadExitPending)
    {
        TRACE_BEGIN_EVENT((uint32)DAVA::Thread::GetCurrentId(), "", "rhi::render_loop");

        if (renderThreadSuspended.load())
        {
            DispatchPlatform::Suspend();
            renderThreadSuspendSyncReached = true;
            renderThreadSuspendSync.Wait();
        }

        DispatchPlatform::CheckSurface();

        TRACE_BEGIN_EVENT((uint32)DAVA::Thread::GetCurrentId(), "", "renderer_wait_core");
        bool frameReady = false;
        while (!frameReady)
        {
            //exit or suspend should leave frame loop
            if (renderThreadExitPending || renderThreadSuspended.load(std::memory_order_relaxed))
                break;

            CheckImmediateCommand();
            frameReady = FrameLoop::FrameReady();

            if (!frameReady)
            {
                framePreparedEvent.Wait();
            }
        }
        TRACE_END_EVENT((uint32)DAVA::Thread::GetCurrentId(), "", "renderer_wait_core");

        if (frameReady)
        {
            FrameLoop::ProcessFrame();
            frameDoneEvent.Signal();
        }

        TRACE_END_EVENT((uint32)DAVA::Thread::GetCurrentId(), "", "rhi::render_loop");
    }

    Logger::Info("[RHI] render-thread finished");
}

void InitializeRenderLoop(uint32 frameCount)
{
    renderThreadFrameCount = frameCount;
    DVASSERT(DispatchPlatform::InitContext);
    DVASSERT(DispatchPlatform::AcquireContext);
    DVASSERT(DispatchPlatform::ReleaseContext);

    if (renderThreadFrameCount) //?ASSERT
    {
        DispatchPlatform::ReleaseContext();

        renderThread = DAVA::Thread::Create(DAVA::Message(&RenderFunc));
        renderThread->SetName("RHI.RENDER_THREAD");
        renderThread->Start();
        //        renderThread->SetPriority(DAVA::Thread::PRIORITY_HIGH); //think - may be threading priority should be somehow configurable
        renderThredStartedSync.Wait();
    }
    else
    {
        DispatchPlatform::InitContext();
    }
}

void SuspendRender()
{
    DVASSERT(!renderThreadSuspended);
    renderThreadSuspended = true;
    if (renderThreadFrameCount)
    {
        while (!renderThreadSuspendSyncReached)
        {
            framePreparedEvent.Signal(); //avoid stall
        }
        renderThreadSuspendSyncReached = false;
    }
    else
    {
        DispatchPlatform::Suspend();
    }

    Logger::Error("Render  Suspended");
}

void ResumeRender()
{
    DVASSERT(renderThreadSuspended);
    Logger::Error("Render Resumed");
    renderThreadSuspended = (false);
    if (renderThreadFrameCount)
    {
        renderThreadSuspendSync.Post();
    }
}

void UninitializeRenderLoop()
{
    if (renderThreadFrameCount) //?ASSERT
    {
        renderThreadExitPending = true;
        if (renderThreadSuspended)
        {
            Logger::Info("RenderThread is suspended. Need resume it to be able to join.");
            ResumeRender();
        }

        framePreparedEvent.Signal();

        Logger::Info("UninitializeRenderThread join begin");
        renderThread->Join();
        Logger::Info("UninitializeRenderThread join end");
    }
}

void IssueImmediateCommand(CommonImpl::ImmediateCommand* command)
{
    if (command->forceImmediate || (renderThreadFrameCount == 0))
    {
        DispatchPlatform::ProcessImmediateCommand(command);
    }
    else
    {
        bool scheduled = false;
        bool executed = false;

        // CRAP: busy-wait
        TRACE_BEGIN_EVENT((uint32)DAVA::Thread::GetCurrentId(), "", "wait_immediate_cmd");

        while (!scheduled)
        {
            pendingImmediateCmdSync.Lock();
            if (pendingImmediateCmd.load() == nullptr)
            {
                pendingImmediateCmd = command;
                scheduled = true;
            }
            pendingImmediateCmdSync.Unlock();
        }

        // CRAP: busy-wait
        while (!executed)
        {
            if (pendingImmediateCmd.load(std::memory_order_relaxed) == nullptr)
            {
                executed = true;
            }
            if (!executed)
            {
                framePreparedEvent.Signal();
                DAVA::Thread::Yield();
            }
        }

        TRACE_END_EVENT((uint32)DAVA::Thread::GetCurrentId(), "", "wait_immediate_cmd");
    }
}
void CheckImmediateCommand()
{
    if (pendingImmediateCmd.load(std::memory_order_relaxed) != nullptr)
    {
        CommonImpl::ImmediateCommand* cmd = pendingImmediateCmd.load();
        if (cmd != nullptr)
        {
            DispatchPlatform::ProcessImmediateCommand(cmd);
            pendingImmediateCmd = nullptr;
        }
    }
}
}
}