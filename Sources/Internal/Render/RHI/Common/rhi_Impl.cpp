
    #include "rhi_Impl.h"

    #if defined(__DAVAENGINE_WIN32__)
        #include "../DX9/rhi_DX9.h"
        #include "../GLES2/rhi_GLES2.h"
    #elif defined(__DAVAENGINE_MACOS__)
        #include "../GLES2/rhi_GLES2.h"
    #elif defined(__DAVAENGINE_IPHONE__)
        #include "../Metal/rhi_Metal.h"
        #include "../GLES2/rhi_GLES2.h"
    #else
    #endif

    #include "Core/Core.h"
    using DAVA::Logger;

namespace rhi
{

uint32   stat_DIP       = InvalidIndex;
uint32   stat_DP        = InvalidIndex;
uint32   stat_SET_PS    = InvalidIndex;
uint32   stat_SET_TEX   = InvalidIndex;
uint32   stat_SET_CB    = InvalidIndex;


static Dispatch _Impl = {0};

void    
SetDispatchTable( const Dispatch& dispatch )
{
    _Impl = dispatch;
}

void
Initialize( Api api, const InitParam& param )
{
    switch( api )
    {
#if defined(__DAVAENGINE_WIN32__)
        case RHI_DX9 :
            dx9_Initialize( param );
            break;

        case RHI_DX11 :
//            dx11_Initialize();
            break;
#endif
            
//#if defined(__DAVAENGINE_WIN32__)  ||  defined(__DAVAENGINE_MACOS__)
        case RHI_GLES2 :
            gles2_Initialize( param );
            break;
//#endif

#if defined(__DAVAENGINE_IPHONE__)
        case RHI_METAL :
            metal_Initialize( param );
            break;
#endif

        default :
        {
            // error 'unsupported' here
        }
    }
}

void
Reset( const ResetParam& param )
{
    (*_Impl.impl_Reset)( param );
}

void
Uninitialize()
{
    (*_Impl.impl_Uninitialize)();
}

void
Present()
{
    (*_Impl.impl_Present)();
}

Api
HostApi()
{
    return (*_Impl.impl_HostApi)();
}

bool
TextureFormatSupported( TextureFormat format )
{
    return (*_Impl.impl_TextureFormatSupported)( format );
}



//////////////////////////////////////////////////////////////////////////

namespace VertexBuffer
{

Handle  
Create( uint32 size, uint32 options )
{
    return (*_Impl.impl_VertexBuffer_Create)( size, options );
}

void
Delete( Handle vb )
{
    return (*_Impl.impl_VertexBuffer_Delete)( vb );
}

bool    
Update( Handle vb, const void* data, uint32 offset, uint32 size )
{
    return (*_Impl.impl_VertexBuffer_Update)( vb, data, offset, size );
}

void*
Map( Handle vb, uint32 offset, uint32 size )
{
    return (*_Impl.impl_VertexBuffer_Map)( vb, offset, size );
}

void
Unmap( Handle vb )
{
    return (*_Impl.impl_VertexBuffer_Unmap)( vb );
}

} // namespace VertexBuffer



//////////////////////////////////////////////////////////////////////////

namespace IndexBuffer
{

Handle  
Create( uint32 size, uint32 options )
{
    return (*_Impl.impl_IndexBuffer_Create)( size, options );
}

void
Delete( Handle vb )
{
    return (*_Impl.impl_IndexBuffer_Delete)( vb );
}

bool    
Update( Handle vb, const void* data, uint32 offset, uint32 size )
{
    return (*_Impl.impl_IndexBuffer_Update)( vb, data, offset, size );
}

void*
Map( Handle vb, uint32 offset, uint32 size )
{
    return (*_Impl.impl_IndexBuffer_Map)( vb, offset, size );
}

void
Unmap( Handle vb )
{
    return (*_Impl.impl_IndexBuffer_Unmap)( vb );
}

} // namespace IndexBuffer


////////////////////////////////////////////////////////////////////////////////

namespace QueryBuffer
{

Handle
Create( uint32 maxObjectCount )
{
    return (*_Impl.impl_QueryBuffer_Create)( maxObjectCount );
}

void
Delete( Handle buf )
{
    (*_Impl.impl_QueryBuffer_Delete)( buf );
}

bool
IsReady( Handle buf, uint32 objectIndex )
{
    return (*_Impl.impl_QueryBuffer_IsReady)( buf, objectIndex );
}

int
Value( Handle buf, uint32 objectIndex )
{
    return (*_Impl.impl_QueryBuffer_Value)( buf, objectIndex );
}

}


////////////////////////////////////////////////////////////////////////////////

namespace 
Texture
{

Handle
Create( const Texture::Descriptor& desc )
{
    return (*_Impl.impl_Texture_Create)( desc );
}

void
Delete( Handle tex )
{
    return (*_Impl.impl_Texture_Delete)( tex );
}

void*
Map( Handle tex, unsigned level, TextureFace face )
{
    return (*_Impl.impl_Texture_Map)( tex, level, face );
}

void
Unmap( Handle tex )
{
    return (*_Impl.impl_Texture_Unmap)( tex );
}

void
Update( Handle tex, const void* data, uint32 level, TextureFace face )
{
    return (*_Impl.impl_Texture_Update)( tex, data, level, face );
}

};



////////////////////////////////////////////////////////////////////////////////

namespace PipelineState
{

Handle
Create( const Descriptor& desc )
{
    return (*_Impl.impl_PipelineState_Create)( desc );
}

void
Delete( Handle ps )
{
    return (*_Impl.impl_PipelineState_Delete)( ps );
}

Handle
CreateVertexConstBuffer( Handle ps, uint32 bufIndex )
{
    return (*_Impl.impl_PipelineState_CreateVertexConstBuffer)( ps, bufIndex );
}

Handle
CreateFragmentConstBuffer( Handle ps, uint32 bufIndex )
{
    return (*_Impl.impl_PipelineState_CreateFragmentConstBuffer)( ps, bufIndex );
}

uint32
VertexConstBufferCount( Handle ps )
{
    return (*_Impl.impl_PipelineState_VertexConstBufferCount)( ps );
}

uint32
VertexConstCount( Handle ps, uint32 bufIndex )
{
    return (*_Impl.impl_PipelineState_VertexConstCount)( ps, bufIndex );
}

bool
GetVertexConstInfo( Handle ps, uint32 bufIndex, uint32 maxCount, ProgConstInfo* info )
{
    return (*_Impl.impl_PipelineState_GetVertexConstInfo)( ps, bufIndex, maxCount, info );
}


uint32
FragmentConstBufferCount( Handle ps )
{
    return (*_Impl.impl_PipelineState_FragmentConstBufferCount)( ps );
}

uint32
FragmentConstCount( Handle ps, uint32 bufIndex )
{
    return (*_Impl.impl_PipelineState_FragmentConstCount)( ps, bufIndex );
}

bool
GetFragmentConstInfo( Handle ps, uint32 bufIndex, uint32 maxCount, ProgConstInfo* info )
{
    return (*_Impl.impl_PipelineState_GetFragmentConstInfo)( ps, bufIndex, maxCount, info );
}

} // namespace PipelineState



//////////////////////////////////////////////////////////////////////////

namespace ConstBuffer
{

uint32
ConstCount( Handle cb )
{
    return (*_Impl.impl_ConstBuffer_ConstCount)( cb );
}

bool
SetConst( Handle cb, uint32 constIndex, uint32 constCount, const float* data )
{
    return (*_Impl.impl_ConstBuffer_SetConst)( cb, constIndex, constCount, data );
}

bool
SetConst( Handle cb, uint32 constIndex, uint32 constSubIndex, const float* data, uint32 dataCount )
{
    return (*_Impl.impl_ConstBuffer_SetConst1fv)( cb, constIndex, constSubIndex, data, dataCount );
}

void
Delete( Handle cb )
{
    return (*_Impl.impl_ConstBuffer_Delete)( cb );
}


} // namespace ConstBuffer



//////////////////////////////////////////////////////////////////////////

namespace
DepthStencilState
{

Handle  
Create( const Descriptor& desc )
{
    return (*_Impl.impl_DepthStencilState_Create)( desc );
}

void
Delete( Handle state )
{
    (*_Impl.impl_DepthStencilState_Delete)( state );
}

}



//////////////////////////////////////////////////////////////////////////

namespace
SamplerState
{

Handle  
Create( const Descriptor& desc )
{
    return (*_Impl.impl_SamplerState_Create)( desc );
}

void
Delete( Handle state )
{
    (*_Impl.impl_SamplerState_Delete)( state );
}

}



//////////////////////////////////////////////////////////////////////////

namespace RenderPass
{

Handle
Allocate( const RenderPassConfig& passDesc, uint32 cmdBufCount, Handle* cmdBuf )
{
    return (*_Impl.impl_Renderpass_Allocate)( passDesc, cmdBufCount, cmdBuf );
}

void
Begin( Handle pass )
{
    return (*_Impl.impl_Renderpass_Begin)( pass );
}

void
End( Handle pass )
{
    return (*_Impl.impl_Renderpass_End)( pass );
}

}



//////////////////////////////////////////////////////////////////////////

namespace CommandBuffer
{

void
Begin( Handle cmdBuf )
{
    (*_Impl.impl_CommandBuffer_Begin)( cmdBuf );
}

void
End( Handle cmdBuf )
{
    (*_Impl.impl_CommandBuffer_End)( cmdBuf );
}

void
SetPipelineState( Handle cmdBuf, Handle ps, uint32 layout )
{
    (*_Impl.impl_CommandBuffer_SetPipelineState)( cmdBuf, ps, layout );
}

void
SetCullMode( Handle cmdBuf, CullMode mode )
{
    (*_Impl.impl_CommandBuffer_SetCullMode)( cmdBuf, mode );
}

void
SetVertexData( Handle cmdBuf, Handle vb, uint32 streamIndex )
{
    (*_Impl.impl_CommandBuffer_SetVertexData)( cmdBuf, vb, streamIndex );
}

void
SetVertexConstBuffer( Handle cmdBuf, uint32 bufIndex, Handle buffer )
{
    (*_Impl.impl_CommandBuffer_SetVertexConstBuffer)( cmdBuf, bufIndex, buffer );
}

void
SetVertexTexture( Handle cmdBuf, uint32 unitIndex, Handle tex )
{
    (*_Impl.impl_CommandBuffer_SetVertexTexture)( cmdBuf, unitIndex, tex );
}

void
SetIndices( Handle cmdBuf, Handle ib )
{
    (*_Impl.impl_CommandBuffer_SetIndices)( cmdBuf, ib );
}

void
SetQueryBuffer( Handle cmdBuf, Handle queryBuf )
{
    (*_Impl.impl_CommandBuffer_SetQueryBuffer)( cmdBuf, queryBuf );
}

void
SetQueryIndex( Handle cmdBuf, uint32 index )
{
    (*_Impl.impl_CommandBuffer_SetQueryIndex)( cmdBuf, index );
}
    
void
SetFragmentConstBuffer( Handle cmdBuf, uint32 bufIndex, Handle buf )
{
    (*_Impl.impl_CommandBuffer_SetFragmentConstBuffer)( cmdBuf, bufIndex, buf );
}

void
SetFragmentTexture( Handle cmdBuf, uint32 unitIndex, Handle tex )
{
    (*_Impl.impl_CommandBuffer_SetFragmentTexture)( cmdBuf, unitIndex, tex );
}

void
SetDepthStencilState( Handle cmdBuf, Handle depthStencilState )
{
    (*_Impl.impl_CommandBuffer_SetDepthStencilState)( cmdBuf, depthStencilState );
}

void
SetSamplerState( Handle cmdBuf, const Handle samplerState )
{
    (*_Impl.impl_CommandBuffer_SetSamplerState)( cmdBuf, samplerState );
}

void
DrawPrimitive( Handle cmdBuf, PrimitiveType type, uint32 count )
{
    (*_Impl.impl_CommandBuffer_DrawPrimitive)( cmdBuf, type, count );
}

void
DrawIndexedPrimitive( Handle cmdBuf, PrimitiveType type, uint32 primCount, uint32 vertexCount, uint32 firstVertex, uint32 startIndex )
{
    (*_Impl.impl_CommandBuffer_DrawIndexedPrimitive)( cmdBuf, type, primCount, vertexCount, firstVertex, startIndex );
}

void
SetMarker( Handle cmdBuf, const char* text )
{
    (*_Impl.impl_CommandBuffer_SetMarker)( cmdBuf, text );
}

} // namespace CommandBuffer



//------------------------------------------------------------------------------

uint32
TextureStride( TextureFormat format, Size2i size, uint32 level )
{
    uint32  stride  = 0;
    Size2i  ext     = TextureExtents( size, level );

    switch( format )
    {
        case TEXTURE_FORMAT_R8G8B8A8 :
        {
            stride = ext.dx * sizeof(uint32);
        }   break;
        
        case TEXTURE_FORMAT_D16 :
        {
            stride = ext.dx * sizeof(uint16);
        }   break;
        
        case TEXTURE_FORMAT_D24S8 :
        {
            stride = ext.dx * sizeof(uint32);
        }   break;
        
        default :
        {
        }
    }

    return stride;
}


//------------------------------------------------------------------------------

Size2i
TextureExtents( Size2i size, uint32 level )
{
    Size2i  sz(size.dx>>level,size.dy>>level);

    if( sz.dx == 0 ) sz.dx = 1;
    if( sz.dy == 0 ) sz.dy = 1;

    return sz;
}


//------------------------------------------------------------------------------

uint32
TextureSize( TextureFormat format, uint32 width, uint32 height, uint32 level )
{
    Size2i  ext = TextureExtents( Size2i(width,height), level );
    uint32  sz  = 0;

    switch( format )
    {
        case TEXTURE_FORMAT_R8G8B8A8 :
        case TEXTURE_FORMAT_R8G8B8X8 :
            sz = ext.dx * ext.dy * sizeof(uint32);
            break;
        
        case TEXTURE_FORMAT_R8G8B8 :
            sz = ext.dx * ext.dy * 3*sizeof(uint8);
            break;

        case TEXTURE_FORMAT_R5G5B5A1 :
        case TEXTURE_FORMAT_R5G6B5 :
            sz = ext.dx * ext.dy * sizeof(uint16);
            break;

        case TEXTURE_FORMAT_R4G4B4A4 :
            sz = ext.dx * ext.dy * sizeof(uint16);
            break;

        case TEXTURE_FORMAT_A16R16G16B16 :
            sz = ext.dx * ext.dy * sizeof(uint16);
            break;

        case TEXTURE_FORMAT_A32R32G32B32 :
            sz = ext.dx * ext.dy * sizeof(float32);
            break;

        case TEXTURE_FORMAT_R8 :
            sz = ext.dx * ext.dy * sizeof(uint8);
            break;

        case TEXTURE_FORMAT_R16 :
            sz = ext.dx * ext.dy * sizeof(uint16);
            break;

        case TEXTURE_FORMAT_DXT1 :
        {
            int ww = ext.dx >> 2;
            int hh = ext.dy >> 2;

            if( !ww )   ww = 1;
            if( !hh )   hh = 1;

            sz = (ww * hh) << 3;
        }   break;

        case TEXTURE_FORMAT_DXT3 :
        case TEXTURE_FORMAT_DXT5 :
        {
            int ww = ext.dx >> 2;
            int hh = ext.dy >> 2;

            if( !ww )   ww = 1;
            if( !hh )   hh = 1;

            sz = (ww * hh) << 4;
        }   break;

        case TEXTURE_FORMAT_PVRTC2_4BPP_RGB :
        case TEXTURE_FORMAT_PVRTC2_4BPP_RGBA :
        {
            uint32  block_h = 4;
            uint32  block_w = 4;
        
            sz = ( (height + block_h - 1) / block_h) * ( (width + block_w - 1) / block_w ) * sizeof(uint64); 
        }   break;

        case TEXTURE_FORMAT_PVRTC2_2BPP_RGB :
        case TEXTURE_FORMAT_PVRTC2_2BPP_RGBA :
        {
            uint32  block_h = 4;
            uint32  block_w = 8;

            sz = ( (height + block_h - 1) / block_h ) * ( (width + block_w - 1) / block_w ) * sizeof(uint64); 
        }   break;

        case TEXTURE_FORMAT_ATC_RGB :
            sz = ((ext.dx+3)/4) * ((ext.dy+3)/4) * 8;
            break;

        case TEXTURE_FORMAT_ATC_RGBA_EXPLICIT :
        case TEXTURE_FORMAT_ATC_RGBA_INTERPOLATED :
            sz = ((ext.dx+3)/4) * ((ext.dy+3)/4) * 16;
            break;

        case TEXTURE_FORMAT_ETC1 :
        case TEXTURE_FORMAT_ETC2_R8G8B8 :
        {
            int ww = ext.dx >> 2;
            int hh = ext.dy >> 2;

            if( !ww )   ww = 1;
            if( !hh )   hh = 1;

            sz = (ww * hh) << 3;
        }   break;

        case TEXTURE_FORMAT_ETC2_R8G8B8A8 :
        case TEXTURE_FORMAT_ETC2_R8G8B8A1 :
        {
            int ww = ext.dx >> 2;
            int hh = ext.dy >> 2;

            if( !ww )   ww = 1;
            if( !hh )   hh = 1;

            sz = (ww * hh) << 4;
        }   break;


        case TEXTURE_FORMAT_EAC_R11_UNSIGNED :
        case TEXTURE_FORMAT_EAC_R11_SIGNED :
        {
            int ww = ext.dx >> 2;
            int hh = ext.dy >> 2;

            if( !ww )   ww = 1;
            if( !hh )   hh = 1;

            sz = (ww * hh) << 3;
        }   break;

        case TEXTURE_FORMAT_EAC_R11G11_UNSIGNED :
        case TEXTURE_FORMAT_EAC_R11G11_SIGNED :
        {
            int ww = ext.dx >> 2;
            int hh = ext.dy >> 2;

            if( !ww )   ww = 1;
            if( !hh )   hh = 1;

            sz = (ww * hh) << 4;
        }   break;

        case TEXTURE_FORMAT_D16 :
            sz = ext.dx * ext.dy * sizeof(uint16);
            break;
                        
        case TEXTURE_FORMAT_D24S8 :
            sz = ext.dx * ext.dy * sizeof(uint32);
            break;

    }

    return sz;
}


//------------------------------------------------------------------------------

uint32
NativeColorRGBA( float red, float green, float blue, float alpha )
{
    uint32  color   = 0;
    int     r       = int(red*255.0f);
    int     g       = int(green*255.0f);
    int     b       = int(blue*255.0f);
    int     a       = int(alpha*255.0f);

    switch( HostApi() )
    {
        case RHI_DX9 :
        case RHI_DX11 :
            color = ((uint32)((((a)&0xFF)<<24)|(((r)&0xFF)<<16)|(((g)&0xFF)<<8)|((b)&0xFF)));
            break;

        case RHI_GLES2 :
            color = ((uint32)((((a)&0xFF)<<24)|(((b)&0xFF)<<16)|(((g)&0xFF)<<8)|((r)&0xFF)));
            break;
        
        case RHI_METAL :
            color = ((uint32)((((a)&0xFF)<<24)|(((r)&0xFF)<<16)|(((g)&0xFF)<<8)|((b)&0xFF)));
            break;
    }

    return color;
}


} //namespace rhi
