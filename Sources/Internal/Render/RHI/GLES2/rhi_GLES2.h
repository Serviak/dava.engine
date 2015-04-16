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


#ifndef __RHI_GLES2_H__
#define __RHI_GLES2_H__

#include "../Common/rhi_Private.h"
#include "../Common/rhi_Impl.h"


namespace rhi
{

void        gles2_Initialize();


namespace VertexBufferGLES2
{
void        SetupDispatch( Dispatch* dispatch );
void        SetToRHI( Handle vb );
}

namespace IndexBufferGLES2
{
void        SetupDispatch( Dispatch* dispatch );
void        SetToRHI( Handle vb );
}

namespace TextureGLES2
{ 
void        SetupDispatch( Dispatch* dispatch );
void        SetToRHI( Handle tex, unsigned unit_i );
void        SetAsRenderTarget( Handle tex );
}

namespace SamplerStateGLES2
{
void        SetupDispatch( Dispatch* dispatch );
void        SetToRHI( Handle hstate );
}


namespace PipelineStateGLES2
{
void        SetupDispatch( Dispatch* dispatch );
void        SetToRHI( Handle ps, uint32 vdeclUID );
}

namespace DepthStencilStateGLES2
{
void        SetupDispatch( Dispatch* dispatch );
void        SetToRHI( Handle hstate );
}
namespace ConstBufferGLES2
{
void        SetupDispatch( Dispatch* dispatch );
void        InitializeRingBuffer( uint32 size );

void        SetToRHI( Handle ps, const void* instData );
const void* Instance( Handle ps );
}

namespace RenderPassGLES2
{
void        SetupDispatch( Dispatch* dispatch );
}

namespace CommandBufferGLES2
{
void        SetupDispatch( Dispatch* dispatch );
}


void        InitializeRenderThread();
void        UninitializeRenderThread();

struct
GLCommand
{
    enum 
    Func
    {
        NOP,
        
        GEN_BUFFERS, 
        BIND_BUFFER, 
        DELETE_BUFFERS, 
        BUFFER_DATA,

        GEN_FRAMEBUFFERS,
        BIND_FRAMEBUFFER,
        FRAMEBUFFER_TEXTURE,
        FRAMEBUFFER_STATUS,
        DRAWBUFFERS,

        GEN_TEXTURES,
        BIND_TEXTURE,
        DELETE_TEXTURES,
        TEX_PARAMETER_I,
        TEX_IMAGE2D,
        GENERATE_MIPMAP,

        CREATE_SHADER,
        SHADER_SOURCE,
        COMPILE_SHADER,
        GET_SHADER_IV,
        GET_SHADER_INFO_LOG,
        GET_PROGRAM_IV,
        GET_ACTIVE_UNIFORM,
        GET_UNIFORM_LOCATION
    };

    Func    func;
    uint64  arg[12];
    int     retval;
    int     status;
};

void     ExecGL( GLCommand* cmd, uint32 cmdCount );

//==============================================================================
}
#endif // __RHI_GLES2_H__

