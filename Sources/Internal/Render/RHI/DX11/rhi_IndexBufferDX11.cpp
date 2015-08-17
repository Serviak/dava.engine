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


    #include "../Common/rhi_Private.h"
    #include "../Common/rhi_Pool.h"
    #include "rhi_DX11.h"

    #include "Debug/DVAssert.h"
    #include "FileSystem/Logger.h"
    using DAVA::Logger;

    #include "_dx11.h"


namespace rhi
{
//==============================================================================


class
IndexBufferDX11_t
{
public:
                            IndexBufferDX11_t();
                            ~IndexBufferDX11_t();

    unsigned        size;
    ID3D11Buffer*   buffer;
    unsigned        isMapped:1;
};

typedef ResourcePool<IndexBufferDX11_t,RESOURCE_VERTEX_BUFFER>  IndexBufferDX11Pool;

RHI_IMPL_POOL(IndexBufferDX11_t,RESOURCE_VERTEX_BUFFER);


IndexBufferDX11_t::IndexBufferDX11_t()
  : size(0),
    buffer(nullptr),
    isMapped(false)
{
}


//------------------------------------------------------------------------------

IndexBufferDX11_t::~IndexBufferDX11_t()
{
}


//==============================================================================


//------------------------------------------------------------------------------

static Handle
dx11_IndexBuffer_Create( const IndexBuffer::Descriptor& desc )
{
    Handle  handle = InvalidHandle;

    DVASSERT(desc.size);
    if( desc.size )
    {
        D3D11_BUFFER_DESC   desc11 = {0};
        ID3D11Buffer*       buf    = nullptr;
        
        desc11.ByteWidth        = desc.size;        
        desc11.Usage            = D3D11_USAGE_DYNAMIC;
        desc11.CPUAccessFlags   = D3D11_CPU_ACCESS_WRITE;
        desc11.BindFlags        = D3D11_BIND_INDEX_BUFFER;                
        desc11.MiscFlags        = 0;
        
        HRESULT hr = _D3D11_Device->CreateBuffer( &desc11, NULL, &buf );

        if( SUCCEEDED(hr) )
        {
            handle = IndexBufferDX11Pool::Alloc();
            IndexBufferDX11_t*    vb = IndexBufferDX11Pool::Get( handle );

            vb->size     = desc.size;
            vb->buffer   = buf;
            vb->isMapped = false;
        }
        else
        {
            Logger::Error( "FAILED to create index-buffer:\n%s\n", D3D11ErrorText(hr) );
        }
    }

    return handle;
}


//------------------------------------------------------------------------------

static void            
dx11_IndexBuffer_Delete( Handle ib )
{
    IndexBufferDX11_t*  self = IndexBufferDX11Pool::Get( ib );

    if( self )
    {
        if( self->buffer )
        {
            self->buffer->Release();
            self->buffer = nullptr;
        }

        self->size = 0;

        IndexBufferDX11Pool::Free( ib );
    }
}


//------------------------------------------------------------------------------
    
static bool
dx11_IndexBuffer_Update( Handle vb, const void* data, unsigned offset, unsigned size )
{
    bool                success = false;
    IndexBufferDX11_t*  self    = IndexBufferDX11Pool::Get( vb );

    DVASSERT(!self->isMapped);

    if( offset+size <= self->size )
    {
        D3D11_MAPPED_SUBRESOURCE    rc = {0};
        
        _D3D11_ImmediateContext->Map( self->buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &rc );
        
        if( rc.pData )
        {            
            memcpy( ((uint8*)(rc.pData))+offset, data, size );            
            _D3D11_ImmediateContext->Unmap( self->buffer, 0 );
            success = true;
        }
    }
    
    return success;
}


//------------------------------------------------------------------------------

static void*
dx11_IndexBuffer_Map( Handle ib, unsigned offset, unsigned size )
{
    void*                       ptr  = nullptr;
    IndexBufferDX11_t*          self = IndexBufferDX11Pool::Get( ib );
    D3D11_MAPPED_SUBRESOURCE    rc   = {0};

    DVASSERT(!self->isMapped);
    _D3D11_ImmediateContext->Map( self->buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &rc );

    if( rc.pData )
    {
        ptr            = rc.pData;
        self->isMapped = true;
    }

    return ((uint8*)ptr)+offset;
}


//------------------------------------------------------------------------------

static void
dx11_IndexBuffer_Unmap( Handle ib )
{
    IndexBufferDX11_t*  self = IndexBufferDX11Pool::Get( ib );
    
    DVASSERT(self->isMapped);
    _D3D11_ImmediateContext->Unmap( self->buffer, 0 );
    self->isMapped = false;
}



//------------------------------------------------------------------------------

namespace IndexBufferDX11
{

void
SetupDispatch( Dispatch* dispatch )
{
    dispatch->impl_IndexBuffer_Create   = &dx11_IndexBuffer_Create;
    dispatch->impl_IndexBuffer_Delete   = &dx11_IndexBuffer_Delete;
    dispatch->impl_IndexBuffer_Update   = &dx11_IndexBuffer_Update;
    dispatch->impl_IndexBuffer_Map      = &dx11_IndexBuffer_Map;
    dispatch->impl_IndexBuffer_Unmap    = &dx11_IndexBuffer_Unmap;
}


void 
SetToRHI( Handle ibh, unsigned offset, ID3D11DeviceContext* context )
{
    IndexBufferDX11_t*  self = IndexBufferDX11Pool::Get( ibh );    
    
///    DVASSERT(!self->isMapped);
    context->IASetIndexBuffer( self->buffer, DXGI_FORMAT_R16_UINT, offset );
}

}


} // namespace rhi
