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

#ifndef __DAVAENGINE_RENDERTARGETFACTORY_H__
#define __DAVAENGINE_RENDERTARGETFACTORY_H__

#include "Base/BaseTypes.h"
#include "Base/BaseMath.h"
#include "Base/BaseObject.h"

#include "Render/RenderTarget/RenderTargetDescriptor.h"
#include "Render/RenderTarget/RenderTarget.h"
#include "Render/RenderTarget/RenderDataReader.h"

namespace DAVA
{

class RenderTargetFactory : public BaseObject
{

public:

    enum GenericAttachmentFlags
    {
        ATTACHMENT_NONE = 0,
        ATTACHMENT_COLOR = 1,
        ATTACHMENT_DEPTH = 2,
        ATTACHMENT_STENCIL = 4,
        ATTACHMENT_COLOR_TEXTURE = 8,
        ATTACHMENT_DEPTH_TEXTURE = 16,
        ATTACHMENT_STENCIL_TEXTURE = 32
    };

protected:

    virtual ~RenderTargetFactory();

public:

    static RenderTargetFactory* Instance();

    static void InitFactory(RenderTargetFactory* factory);
    static void ReleaseFactory();

    virtual RenderTarget* CreateRenderTarget(const RenderTargetDescriptor& rtDesc) = 0;
    virtual RenderDataReader* GetRenderDataReader() = 0;

    RenderTarget* CreateRenderTarget(uint32 flags,
                                     uint32 width,
                                     uint32 height);

    RenderTarget* CreateRenderTarget(uint32 flags,
                                     uint32 width,
                                     uint32 height,
                                     FramebufferDescriptor::PreRenderAction colorPreRenderAction,
                                     FramebufferDescriptor::PostRenderAction colorPostRenderAction);

    RenderTarget* CreateRenderTarget(uint32 flags,
                                     uint32 width,
                                     uint32 height,
                                     FramebufferDescriptor::PreRenderAction colorPreRenderAction,
                                     FramebufferDescriptor::PostRenderAction colorPostRenderAction,
                                     FramebufferDescriptor::PreRenderAction depthPreRenderAction,
                                     FramebufferDescriptor::PostRenderAction depthPostRenderAction,
                                     FramebufferDescriptor::PreRenderAction stencilPreRenderAction,
                                     FramebufferDescriptor::PostRenderAction stencilPostRenderAction);

    RenderTarget* CreateRenderTarget(GenericAttachmentFlags colorAttachmentType,
                                     GenericAttachmentFlags depthAttachmentType,
                                     GenericAttachmentFlags stencilAttachmentType,
                                     uint32 width,
                                     uint32 height,
                                     FramebufferDescriptor::FramebufferFormat colorFormat,
                                     FramebufferDescriptor::FramebufferFormat depthFormat,
                                     FramebufferDescriptor::FramebufferFormat stencilFormat,
                                     FramebufferDescriptor::PreRenderAction colorPreRenderAction,
                                     FramebufferDescriptor::PostRenderAction colorPostRenderAction,
                                     FramebufferDescriptor::PreRenderAction depthPreRenderAction,
                                     FramebufferDescriptor::PostRenderAction depthPostRenderAction,
                                     FramebufferDescriptor::PreRenderAction stencilPreRenderAction,
                                     FramebufferDescriptor::PostRenderAction stencilPostRenderAction);

    void ConstructGenericTargetDescription(uint32 flags,
                                           uint32 width,
                                           uint32 height,
                                           RenderTargetDescriptor& outDesc);


    void ConstructGenericTargetDescription(uint32 flags,
                                           uint32 width,
                                           uint32 height,
                                           FramebufferDescriptor::PreRenderAction colorPreRenderAction,
                                           FramebufferDescriptor::PostRenderAction colorPostRenderAction,
                                           RenderTargetDescriptor& outDesc);

    void ConstructGenericTargetDescription(uint32 flags,
                                           uint32 width,
                                           uint32 height,
                                           FramebufferDescriptor::PreRenderAction colorPreRenderAction,
                                           FramebufferDescriptor::PostRenderAction colorPostRenderAction,
                                           FramebufferDescriptor::PreRenderAction depthPreRenderAction,
                                           FramebufferDescriptor::PostRenderAction depthPostRenderAction,
                                           FramebufferDescriptor::PreRenderAction stencilPreRenderAction,
                                           FramebufferDescriptor::PostRenderAction stencilPostRenderAction,
                                           RenderTargetDescriptor& outDesc);


    void ConstructGenericTargetDescription(GenericAttachmentFlags colorAttachmentType,
                                           GenericAttachmentFlags depthAttachmentType,
                                           GenericAttachmentFlags stencilAttachmentType,
                                           uint32 width,
                                           uint32 height,
                                           FramebufferDescriptor::FramebufferFormat colorFormat,
                                           FramebufferDescriptor::FramebufferFormat depthFormat,
                                           FramebufferDescriptor::FramebufferFormat stencilFormat,
                                           FramebufferDescriptor::PreRenderAction colorPreRenderAction,
                                           FramebufferDescriptor::PostRenderAction colorPostRenderAction,
                                           FramebufferDescriptor::PreRenderAction depthPreRenderAction,
                                           FramebufferDescriptor::PostRenderAction depthPostRenderAction,
                                           FramebufferDescriptor::PreRenderAction stencilPreRenderAction,
                                           FramebufferDescriptor::PostRenderAction stencilPostRenderAction,
                                           RenderTargetDescriptor& outDesc);

protected:

    static RenderTargetFactory* instance;
};

};

#endif /* defined(__DAVAENGINE_RENDERTARGETFACTORY_H__) */
