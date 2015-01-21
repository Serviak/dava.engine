//==============================================================================
//
//  
//
//==============================================================================
//
//  externals:

    #include "../RHI/rhi_Pool.h"
    #include "rhi_DX9.h"

    #include "../rhi_Base.h"
    #include "../rhi_Type.h"

    #include "Debug/DVAssert.h"
    #include "FileSystem/Logger.h"
    using DAVA::Logger;
    #include "Core/Core.h"
    #include "Base/Profiler.h"

    #include "_dx9.h"



namespace rhi
{
//==============================================================================

static bool _ResetPending   = false;


#define DX9_CALL(code,name) \
{ \
    HRESULT hr = code; \
\
    if( FAILED(hr) ) \
        Logger::Error( "%s failed:\n%s\n", name, D3D9ErrorText(hr) ); \
} \


//------------------------------------------------------------------------------

enum
CommandDX9
{
    DX9__BEGIN,
    DX9__END,
    DX9__CLEAR,

    DX9__SET_STREAM_SOURCE,
    DX9__SET_INDICES,

    DX9__SET_PIPELINE_STATE,
    DX9__SET_VERTEX_PROG_CONST_BUFFER,
    DX9__SET_FRAGMENT_PROG_CONST_BUFFER,
    DX9__SET_TEXTURE,

    DX9__DRAW_PRIMITIVE,
    DX9__DRAW_INDEXED_PRIMITIVE,


    DX9__NOP
};


class
CommandBuffer_t
{
public:
                CommandBuffer_t();
                ~CommandBuffer_t();

    void        Begin();
    void        End();
    void        Replay();

    void        Command( uint64 cmd );
    void        Command( uint64 cmd, uint64 arg1 );
    void        Command( uint64 cmd, uint64 arg1, uint64 arg2 );
    void        Command( uint64 cmd, uint64 arg1, uint64 arg2, uint64 arg3 );
    void        Command( uint64 cmd, uint64 arg1, uint64 arg2, uint64 arg3, uint64 arg4 );
    void        Command( uint64 cmd, uint64 arg1, uint64 arg2, uint64 arg3, uint64 arg4, uint64 arg5 );
    void        Command( uint64 cmd, uint64 arg1, uint64 arg2, uint64 arg3, uint64 arg4, uint64 arg5, uint64 arg6 );


private:

    static const uint64   EndCmd/* = 0xFFFFFFFF*/;

    std::vector<uint64> _cmd;
};

typedef Pool<CommandBuffer_t>   CommandBufferPool;
    
const uint64   CommandBuffer_t::EndCmd = 0xFFFFFFFF;



namespace CommandBuffer
{

//------------------------------------------------------------------------------

Handle
Default()
{
    static Handle cb = 0;

    if( !cb )
        cb = CommandBufferPool::Alloc();

    return cb;
}


//------------------------------------------------------------------------------

void
Begin( Handle cmdBuf )
{
    CommandBufferPool::Get(cmdBuf)->Command( DX9__BEGIN );
}


//------------------------------------------------------------------------------

void
End( Handle cmdBuf )
{
    CommandBufferPool::Get(cmdBuf)->Command( DX9__END );
}


//------------------------------------------------------------------------------

void
Clear( Handle cmdBuf )
{
    CommandBufferPool::Get(cmdBuf)->Command( DX9__CLEAR, 0xFF809080, nonaliased_cast<float,uint32>(1.0f) );
}


//------------------------------------------------------------------------------

void
SetPipelineState( Handle cmdBuf, Handle ps )
{
    CommandBufferPool::Get(cmdBuf)->Command( DX9__SET_PIPELINE_STATE, ps );
}


//------------------------------------------------------------------------------

void
SetVertexData( Handle cmdBuf, Handle vb, uint32 streamIndex )
{
    CommandBufferPool::Get(cmdBuf)->Command( DX9__SET_STREAM_SOURCE, vb, streamIndex );
}


//------------------------------------------------------------------------------

void
SetVertexConstBuffer( Handle cmdBuf, uint32 bufIndex, Handle buffer )
{
//    L_ASSERT(buffer);
    DVASSERT(bufIndex < MAX_CONST_BUFFER_COUNT);
    
    if( buffer != InvalidIndex )
        CommandBufferPool::Get(cmdBuf)->Command( DX9__SET_VERTEX_PROG_CONST_BUFFER, bufIndex, (uint64)(buffer) );
}


//------------------------------------------------------------------------------

void
SetVertexTexture( Handle cmdBuf, uint32 unitIndex, Handle tex )
{
}


//------------------------------------------------------------------------------

void
SetIndices( Handle cmdBuf, Handle ib )
{
    CommandBufferPool::Get(cmdBuf)->Command( DX9__SET_INDICES, ib );
}


//------------------------------------------------------------------------------

void
SetFragmentConstBuffer( Handle cmdBuf, uint32 bufIndex, Handle buffer )
{
//    L_ASSERT(buffer);
    DVASSERT(bufIndex < MAX_CONST_BUFFER_COUNT);
    
    if( buffer != InvalidIndex )
        CommandBufferPool::Get(cmdBuf)->Command( DX9__SET_FRAGMENT_PROG_CONST_BUFFER, bufIndex, (uint64)(buffer) );
}


//------------------------------------------------------------------------------

void
SetFragmentTexture( Handle cmdBuf, uint32 unitIndex, Handle tex )
{
//    L_ASSERT(tex);
    CommandBufferPool::Get(cmdBuf)->Command( DX9__SET_TEXTURE, unitIndex, (uint64)(tex) );
}


//------------------------------------------------------------------------------

void
SetDepthStencilState( Handle cmdBuf, const DepthStencilState& bs )
{
}


//------------------------------------------------------------------------------

void
SetSamplerState( Handle cmdBuf, const SamplerState& ss )
{
}


//------------------------------------------------------------------------------

void
DrawPrimitive( Handle cmdBuf, PrimitiveType type, uint32 count )
{
    D3DPRIMITIVETYPE    type9   = D3DPT_TRIANGLELIST;

    switch( type )
    {
        case PRIMITIVE_TRIANGLELIST :
            type9 = D3DPT_TRIANGLELIST;
            break;
    }

    CommandBufferPool::Get(cmdBuf)->Command( DX9__DRAW_PRIMITIVE, type9, count );
}


//------------------------------------------------------------------------------

void
DrawIndexedPrimitive( Handle cmdBuf, PrimitiveType type, uint32 count )
{
    unsigned            v_cnt   = 0;
    D3DPRIMITIVETYPE    type9   = D3DPT_TRIANGLELIST;

    switch( type )
    {
        case PRIMITIVE_TRIANGLELIST :
            type9 = D3DPT_TRIANGLELIST;
            v_cnt = count*3;
            break;
    }

    CommandBufferPool::Get(cmdBuf)->Command( DX9__DRAW_INDEXED_PRIMITIVE, type9, count, v_cnt );
}


} // namespace CommandBuffer






CommandBuffer_t::CommandBuffer_t()
{
}


//------------------------------------------------------------------------------


CommandBuffer_t::~CommandBuffer_t()
{
}


//------------------------------------------------------------------------------

void
CommandBuffer_t::Begin()
{
    _cmd.clear();
}


//------------------------------------------------------------------------------

void        
CommandBuffer_t::End()
{
    _cmd.push_back( EndCmd );
}


//------------------------------------------------------------------------------

void
CommandBuffer_t::Command( uint64 cmd )
{
    _cmd.push_back( cmd );
}


//------------------------------------------------------------------------------

void
CommandBuffer_t::Command( uint64 cmd, uint64 arg1 )
{
    _cmd.resize( _cmd.size()+1+1 );

    std::vector<uint64>::iterator   b = _cmd.end() - (1+1);

    b[0] = cmd; 
    b[1] = arg1; 
}


//------------------------------------------------------------------------------

void
CommandBuffer_t::Command( uint64 cmd, uint64 arg1, uint64 arg2 )
{
    _cmd.resize( _cmd.size()+1+2 );

    std::vector<uint64>::iterator   b = _cmd.end() - (1+2);

    b[0] = cmd; 
    b[1] = arg1; 
    b[2] = arg2; 
}


//------------------------------------------------------------------------------

void
CommandBuffer_t::Command( uint64 cmd, uint64 arg1, uint64 arg2, uint64 arg3 )
{
    _cmd.resize( _cmd.size()+1+3 );

    std::vector<uint64>::iterator   b = _cmd.end() - (1+3);

    b[0] = cmd; 
    b[1] = arg1; 
    b[2] = arg2; 
    b[3] = arg3; 
}


//------------------------------------------------------------------------------

void
CommandBuffer_t::Command( uint64 cmd, uint64 arg1, uint64 arg2, uint64 arg3, uint64 arg4 )
{
    _cmd.resize( _cmd.size()+1+4 );

    std::vector<uint64>::iterator   b = _cmd.end() - (1+4);

    b[0] = cmd; 
    b[1] = arg1; 
    b[2] = arg2; 
    b[3] = arg3; 
    b[4] = arg4; 
}


//------------------------------------------------------------------------------

void
CommandBuffer_t::Command( uint64 cmd, uint64 arg1, uint64 arg2, uint64 arg3, uint64 arg4, uint64 arg5 )
{
    _cmd.resize( _cmd.size()+1+5 );

    std::vector<uint64>::iterator   b = _cmd.end() - (1+5);

    b[0] = cmd; 
    b[1] = arg1; 
    b[2] = arg2; 
    b[3] = arg3; 
    b[4] = arg4; 
    b[5] = arg5; 
}


//------------------------------------------------------------------------------

inline void
CommandBuffer_t::Command( uint64 cmd, uint64 arg1, uint64 arg2, uint64 arg3, uint64 arg4, uint64 arg5, uint64 arg6 )
{
    _cmd.resize( _cmd.size()+1+6 );

    std::vector<uint64>::iterator   b = _cmd.end() - (1+6);

    b[0] = cmd; 
    b[1] = arg1; 
    b[2] = arg2; 
    b[3] = arg3; 
    b[4] = arg4; 
    b[5] = arg5; 
    b[6] = arg6; 
}


//------------------------------------------------------------------------------

void        
CommandBuffer_t::Replay()
{
SCOPED_FUNCTION_TIMING();
    Handle  cur_pipelinestate = InvalidHandle;

    for( std::vector<uint64>::const_iterator c=_cmd.begin(),c_end=_cmd.end(); c!=c_end; ++c )
    {
        const uint64                        cmd = *c;
        std::vector<uint64>::const_iterator arg = c+1;

        if( cmd == EndCmd )
            break;

        switch( cmd )
        {
            case DX9__BEGIN :
            {
                DX9_CALL(_D3D9_Device->BeginScene(),"BeginScene");
            }   break;
            
            case DX9__END :
            {
                DX9_CALL(_D3D9_Device->EndScene(),"EndScene");
            }   break;
            
            case DX9__CLEAR :
            {
                uint32  clr = uint32(arg[0]);
                float   z   = nonaliased_cast<uint32,float>(uint32(arg[1]));
                
                DX9_CALL(_D3D9_Device->Clear( 0,NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER, clr, z, 0 ),"Clear");
                c += 2;    
            }   break;
            
            case DX9__SET_STREAM_SOURCE :
            {
                DVASSERT(cur_pipelinestate != InvalidHandle);
                unsigned    stride = PipelineStateDX9::VertexLayoutStride( cur_pipelinestate );

                VertexBufferDX9::SetToRHI( (Handle)(arg[0]), 0, 0, stride );
                c += 2;
            }   break;
            
            case DX9__SET_INDICES :
            {
                IndexBufferDX9::SetToRHI( (Handle)(arg[0]) );
                c += 1;
            }   break;

            case DX9__SET_PIPELINE_STATE :
            {
                cur_pipelinestate = (Handle)(arg[0]);
                PipelineStateDX9::SetToRHI( cur_pipelinestate );
                c += 1;
            }   break;
            
            case DX9__SET_VERTEX_PROG_CONST_BUFFER :
            {
                ConstBufferDX9::SetToRHI( (Handle)(arg[1]) );
                c += 1;
            }   break;

            case DX9__SET_FRAGMENT_PROG_CONST_BUFFER :
            {
                ConstBufferDX9::SetToRHI( (Handle)(arg[1]) );
                c += 1;
            }   break;

            case DX9__SET_TEXTURE :
            {
                TextureDX9::SetToRHI( (Handle)(arg[1]), (unsigned)(arg[0]) );
                c += 2;
            }   break;
            
            case DX9__DRAW_PRIMITIVE :
            {
                DX9_CALL(_D3D9_Device->DrawPrimitive( (D3DPRIMITIVETYPE)(arg[0]), /*base_vertex*/0, UINT(arg[1]) ),"DrawPrimitive");
                c += 2;    
            }   break;
            
            case DX9__DRAW_INDEXED_PRIMITIVE :
            {
                DX9_CALL(_D3D9_Device->DrawIndexedPrimitive( (D3DPRIMITIVETYPE)(arg[0]), /*base_vertex*/0, 0,UINT(arg[2]), 0, UINT(arg[1]) ),"DrawIndexedPrimitive");
                c += 3;    
            }   break;

        }
    }

    _cmd.clear();
}



//------------------------------------------------------------------------------

void
Present()
{
    DVASSERT(_D3D9_Device)

    Pool<CommandBuffer_t>::Get(CommandBuffer::Default())->Replay();


//-    bool    success = false;
    HRESULT hr;

    if( _ResetPending )
    {
        hr = _D3D9_Device->TestCooperativeLevel();

        if( hr == D3DERR_DEVICENOTRESET )
        {
///            reset( Size2i(_present_param->BackBufferWidth,_present_param->BackBufferHeight) );

            _ResetPending = false;
        }
        else
        {
            ::Sleep( 100 );
        }
    }
    else
    {
        hr = _D3D9_Device->Present( NULL, NULL, NULL, NULL );

        if( FAILED(hr) )
            Logger::Error( "present() failed:\n%s\n", D3D9ErrorText(hr) );

        if( hr == D3DERR_DEVICELOST )
            _ResetPending = true;
///        _reset_stats();    
    }    
}



//==============================================================================
} // namespace rhi

