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

#include "Renderer.h"
#include "Render/PixelFormatDescriptor.h"
#include "Render/RHI/rhi_ShaderCache.h"
#include "Render/ShaderCache.h"
#include "Render/Material/FXCache.h"
#include "Render/DynamicBufferAllocator.h"
#include "Render/GPUFamilyDescriptor.h"

namespace DAVA
{
namespace Renderer
{

namespace //for private variables
{
    bool ininialized = false;
    rhi::Api api;
    int32 desiredFPS = 0;
    
    RenderOptions renderOptions;
    RenderCaps renderCaps;
    DynamicBindings dynamicBindings;

    int32 framebufferWidth;
    int32 framebufferHeight;

    ScreenShotCallbackDelegate * screenshotCallback = nullptr;
}

void Initialize(rhi::Api _api, const rhi::InitParam & params, int32 width, int32 height)
{
    DVASSERT(!ininialized);

    api = _api;
    
    framebufferWidth = width;
    framebufferHeight = height;
    
    rhi::Initialize(api, params);
    rhi::ShaderCache::Initialize();
    ShaderDescriptorCache::Initialize();
    FXCache::Initialize();
    PixelFormatDescriptor::InitializePixelFormatDescriptors();
    GPUFamilyDescriptor::SetupGPUParameters();
    
    ininialized = true;
}
void Uninitialize()
{
    DVASSERT(ininialized);
            
    FXCache::Uninitialize();
    ShaderDescriptorCache::Uninitialize();
    rhi::ShaderCache::Unitialize();
    rhi::Uninitialize();
    ininialized = false;
}

void Reset(int32 _framebufferWidth, int32 _framebufferHeight)
{
    framebufferWidth = _framebufferWidth;
    framebufferHeight = _framebufferHeight;
}

bool IsDeviceLost()
{
    DVASSERT(ininialized);
    return false;
}

rhi::Api GetAPI()
{
    DVASSERT(ininialized);
    return api;
}

int32 GetDesiredFPS()
{
    return desiredFPS;
}

void SetDesiredFPS(int32 fps)
{
    desiredFPS = fps;
}

RenderOptions *GetOptions()
{
    DVASSERT(ininialized);
    return &renderOptions;
}

const RenderCaps & GetCaps()
{
    return renderCaps;
}

DynamicBindings& GetDynamicBindings()
{
    return dynamicBindings;
}

int32 GetFramebufferWidth()
{
    return framebufferWidth;
}

int32 GetFramebufferHeight()
{
    return framebufferHeight;
}

void RequestGLScreenShot(ScreenShotCallbackDelegate *_screenShotCallback)
{
    screenshotCallback = _screenShotCallback;
    //RHI_COMPLETE
}

void BeginFrame()
{
    DynamicBufferAllocator::BeginFrame();
}
void EndFrame()
{
    DynamicBufferAllocator::EndFrame();
    rhi::Present();
}


}
}