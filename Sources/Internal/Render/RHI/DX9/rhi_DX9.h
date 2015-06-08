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


#ifndef __RHI_DX9_H__
#define __RHI_DX9_H__

#include "../rhi_Public.h"
#include "../Common/rhi_Private.h"
#include "../Common/rhi_Impl.h"


namespace rhi
{

void        dx9_Initialize( const InitParam& param );


namespace VertexBufferDX9
{
void        SetupDispatch( Dispatch* dispatch );
void        SetToRHI( Handle vb, unsigned stream_i, unsigned offset, unsigned stride );
}

namespace IndexBufferDX9
{
void        SetupDispatch( Dispatch* dispatch );
void        SetToRHI( Handle vb );
}

namespace QueryBufferDX9
{
void        SetupDispatch( Dispatch* dispatch );

void        BeginQuery( Handle buf, uint32 objectIndex );
void        EndQuery( Handle buf, uint32 objectIndex );
}


namespace PipelineStateDX9
{
void        SetupDispatch( Dispatch* dispatch );
unsigned    VertexLayoutStride( Handle ps );
void        SetToRHI( Handle ps, uint32 layoutUID );
}

namespace ConstBufferDX9
{
void        SetupDispatch( Dispatch* dispatch );
void        InitializeRingBuffer( uint32 size );
const void* InstData( Handle cb );
void        SetToRHI( Handle cb, const void* instData );
void        InvalidateAllConstBufferInstances();
}

namespace TextureDX9
{
void        SetupDispatch( Dispatch* dispatch );
void        SetToRHI( Handle tex, unsigned unitIndex );
void        SetAsRenderTarget( Handle tex );
}


namespace DepthStencilStateDX9
{
void        SetupDispatch( Dispatch* dispatch );
void        SetToRHI( Handle state );
}

namespace SamplerStateDX9
{
void        SetupDispatch( Dispatch* dispatch );
void        SetToRHI( Handle state );
}


namespace RenderPassDX9
{
void        SetupDispatch( Dispatch* dispatch );
}

namespace CommandBufferDX9
{
void        SetupDispatch( Dispatch* dispatch );
}


#define DX9_CALL(code,name) \
{ \
    HRESULT hr = code; \
\
    if( FAILED(hr) ) \
    { \
        Logger::Error("%s failed (%08X):\n%s\n", name, hr, D3D9ErrorText(hr)); \
    } \
} \


//==============================================================================
}
#endif // __RHI_DX9_H__

