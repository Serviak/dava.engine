/*==================================================================================
    Copyright(c) 2008, binaryzebra
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

#include <Debug/DVAssert.h>

#include "IOLoop.h"

namespace DAVA
{

IOLoop::IOLoop(bool useDefaultIOLoop) : loop()
                                      , actualLoop(NULL)
                                      , useDefaultLoop(useDefaultIOLoop)
{
    if(!useDefaultIOLoop)
    {
        uv_loop_init(&loop);
        actualLoop = &loop;
    }
    else
    {
        actualLoop = uv_default_loop();
    }
    actualLoop->data = this;
}

IOLoop::~IOLoop()
{
    if(!useDefaultLoop)
    {
        DVVERIFY(0 == uv_loop_close(actualLoop));
    }
}

int32 IOLoop::Run(eRunMode runMode)
{
    static const uv_run_mode modes[] = {
        UV_RUN_DEFAULT,
        UV_RUN_ONCE,
        UV_RUN_NOWAIT
    };
    DVASSERT(RUN_DEFAULT == runMode || RUN_ONCE == runMode || RUN_NOWAIT == runMode);
    return uv_run(actualLoop, modes[runMode]);
}

void IOLoop::Stop()
{
    uv_stop(actualLoop);
}

bool IOLoop::IsAlive() const
{
    return uv_loop_alive(actualLoop) != 0;
}

}   // namespace DAVA
