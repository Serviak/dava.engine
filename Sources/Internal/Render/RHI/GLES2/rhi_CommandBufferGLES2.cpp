﻿
    #include "../Common/rhi_Pool.h"
    #include "rhi_GLES2.h"
    #include "rhi_ProgGLES2.h"

    #include "../Common/rhi_Private.h"
    #include "../Common/dbg_StatSet.h"

    #include "Debug/DVAssert.h"
    #include "FileSystem/Logger.h"
    using DAVA::Logger;
    #include "Platform/Thread.h"
    #include "Thread/Semaphore.h"
    #include "Debug/Profiler.h"

    #include "_gl.h"

    #define USE_RENDER_THREAD 0

namespace rhi
{

enum
CommandGLES2
{
    GLES2__BEGIN,
    GLES2__END,

    GLES2__SET_VERTEX_DATA,
    GLES2__SET_INDICES,
    GLES2__SET_QUERY_BUFFER,
    GLES2__SET_QUERY_INDEX,

    GLES2__SET_PIPELINE_STATE,
    GLES2__SET_CULL_MODE,
    GLES2__SET_SCISSOR_RECT,
    GLES2__SET_VIEWPORT,
    GLES2__SET_VERTEX_PROG_CONST_BUFFER,
    GLES2__SET_FRAGMENT_PROG_CONST_BUFFER,
    GLES2__SET_VERTEX_TEXTURE,
    GLES2__SET_FRAGMENT_TEXTURE,

    GLES2__SET_DEPTHSTENCIL_STATE,
    GLES2__SET_SAMPLER_STATE,

    GLES2__DRAW_PRIMITIVE,
    GLES2__DRAW_INDEXED_PRIMITIVE,

    GLES2__SET_MARKER,


    GLES2__NOP
};

struct
RenderPassGLES2_t
{
    std::vector<Handle> cmdBuf;
    int                 priority;
};


struct
CommandBufferGLES2_t
{
public:
                CommandBufferGLES2_t();
                ~CommandBufferGLES2_t();

    void        Begin();
    void        End();
    void        Execute();

    void        Command( uint64 cmd );
    void        Command( uint64 cmd, uint64 arg1 );
    void        Command( uint64 cmd, uint64 arg1, uint64 arg2 );
    void        Command( uint64 cmd, uint64 arg1, uint64 arg2, uint64 arg3 );
    void        Command( uint64 cmd, uint64 arg1, uint64 arg2, uint64 arg3, uint64 arg4 );
    void        Command( uint64 cmd, uint64 arg1, uint64 arg2, uint64 arg3, uint64 arg4, uint64 arg5 );
    void        Command( uint64 cmd, uint64 arg1, uint64 arg2, uint64 arg3, uint64 arg4, uint64 arg5, uint64 arg6 );


    static const uint64   EndCmd/* = 0xFFFFFFFF*/;

    std::vector<uint64> _cmd;

    RenderPassConfig    passCfg;
    uint32              isFirstInPass:1;
    uint32              isLastInPass:1;
};

typedef Pool<CommandBufferGLES2_t,RESOURCE_COMMAND_BUFFER>  CommandBufferPool;
typedef Pool<RenderPassGLES2_t,RESOURCE_RENDER_PASS>        RenderPassPool;

RHI_IMPL_POOL(CommandBufferGLES2_t,RESOURCE_COMMAND_BUFFER);
RHI_IMPL_POOL(RenderPassGLES2_t,RESOURCE_RENDER_PASS);
    
const uint64   CommandBufferGLES2_t::EndCmd = 0xFFFFFFFF;

static const unsigned       _CmdQueueCount              = 3;
static unsigned             _CurCmdQueueIndex           = 0;
static std::vector<Handle>  _CmdQueue[_CmdQueueCount];
static std::vector<uint32>  _RenderQueue;
static uint32               _CurRenderQueueSize         = 0;
static DAVA::Spinlock       _CmdQueueSync;

static bool                 _CmdBufIsBeingExecuted      = false;
static DAVA::Spinlock       _CmdBufIsBeingExecutedSync;

static GLCommand*           _PendingImmediateCmd        = nullptr;
static uint32               _PendingImmediateCmdCount   = 0;
#if USE_RENDER_THREAD
static bool                 _ImmediateCmdAllowed        = true;
#endif
static DAVA::Spinlock       _PendingImmediateCmdSync;

static bool                 _RenderThreadExitPending    = false;
static DAVA::Spinlock       _RenderThreadExitSync;
static DAVA::Semaphore      _RenderThredStartedSync     (1);

#if USE_RENDER_THREAD
static DAVA::Thread*        RenderThread;
#endif

static void _ExecGL( GLCommand* command, uint32 cmdCount );



static Handle
gles2_RenderPass_Allocate( const RenderPassConfig& passConf, uint32 cmdBufCount, Handle* cmdBuf )
{
    DVASSERT(cmdBufCount);

    Handle              handle  = RenderPassPool::Alloc();
    RenderPassGLES2_t*  pass    = RenderPassPool::Get( handle );

    pass->cmdBuf.resize( cmdBufCount );
    pass->priority = passConf.priority;

    for( unsigned i=0; i!=cmdBufCount; ++i )
    {
        Handle                  h  = CommandBufferPool::Alloc();
        CommandBufferGLES2_t*   cb = CommandBufferPool::Get( h );

        cb->passCfg         = passConf;
        cb->isFirstInPass   = i == 0;
        cb->isLastInPass    = i == cmdBufCount - 1;
        
        pass->cmdBuf[i] = h;
        cmdBuf[i]       = h;
    }

    return handle;
}

static void
gles2_RenderPass_Begin( Handle pass )
{
    _CmdQueue[_CurCmdQueueIndex].push_back( pass );
}

static void
gles2_RenderPass_End( Handle pass )
{
}

namespace RenderPassGLES2
{

void
SetupDispatch( Dispatch* dispatch )
{
    dispatch->impl_Renderpass_Allocate  = &gles2_RenderPass_Allocate;
    dispatch->impl_Renderpass_Begin     = &gles2_RenderPass_Begin;
    dispatch->impl_Renderpass_End       = &gles2_RenderPass_End;
}

}



//------------------------------------------------------------------------------

static Handle
gles2_CommandBuffer_Allocate()
{
    Handle cb = CommandBufferPool::Alloc();

    return cb;
}


//------------------------------------------------------------------------------

static void
gles2_CommandBuffer_Submit( Handle cb )
{
    _CmdQueue[_CurCmdQueueIndex].push_back( cb );
}


//------------------------------------------------------------------------------

static void
gles2_CommandBuffer_Begin( Handle cmdBuf )
{
    CommandBufferPool::Get(cmdBuf)->Command( GLES2__BEGIN );
}


//------------------------------------------------------------------------------

static void
gles2_CommandBuffer_End( Handle cmdBuf )
{
    CommandBufferPool::Get(cmdBuf)->Command( GLES2__END );
}


//------------------------------------------------------------------------------

static void
gles2_CommandBuffer_SetPipelineState( Handle cmdBuf, Handle ps, uint32 vdecl )
{
    CommandBufferPool::Get(cmdBuf)->Command( GLES2__SET_PIPELINE_STATE, ps, vdecl );
}


//------------------------------------------------------------------------------

static void
gles2_CommandBuffer_SetCullMode( Handle cmdBuf, CullMode mode )
{
    CommandBufferPool::Get(cmdBuf)->Command( GLES2__SET_CULL_MODE, mode );
}


//------------------------------------------------------------------------------

void
gles2_CommandBuffer_SetScissorRect( Handle cmdBuf, ScissorRect rect )
{
    CommandBufferPool::Get(cmdBuf)->Command( GLES2__SET_SCISSOR_RECT, rect.x, rect.y, rect.width, rect.height );
}


//------------------------------------------------------------------------------

void
gles2_CommandBuffer_SetViewport( Handle cmdBuf, Viewport vp )
{
    CommandBufferPool::Get(cmdBuf)->Command( GLES2__SET_VIEWPORT, vp.x, vp.y, vp.width, vp.height );
}


//------------------------------------------------------------------------------

static void
gles2_CommandBuffer_SetVertexData( Handle cmdBuf, Handle vb, uint32 streamIndex )
{
    CommandBufferPool::Get(cmdBuf)->Command( GLES2__SET_VERTEX_DATA, vb, streamIndex );
}


//------------------------------------------------------------------------------

static void
gles2_CommandBuffer_SetVertexConstBuffer( Handle cmdBuf, uint32 bufIndex, Handle buffer )
{
//    L_ASSERT(buffer);
    DVASSERT(bufIndex < MAX_CONST_BUFFER_COUNT);
    
    if( buffer != InvalidHandle )
        CommandBufferPool::Get(cmdBuf)->Command( GLES2__SET_VERTEX_PROG_CONST_BUFFER, bufIndex, buffer, (uint64)(ConstBufferGLES2::Instance(buffer)) );
}


//------------------------------------------------------------------------------

static void
gles2_CommandBuffer_SetVertexTexture( Handle cmdBuf, uint32 unitIndex, Handle tex )
{
    if( tex != InvalidHandle )
        CommandBufferPool::Get(cmdBuf)->Command( GLES2__SET_VERTEX_TEXTURE, unitIndex, tex );
}


//------------------------------------------------------------------------------

static void
gles2_CommandBuffer_SetIndices( Handle cmdBuf, Handle ib )
{
    CommandBufferPool::Get(cmdBuf)->Command( GLES2__SET_INDICES, ib );
}


//------------------------------------------------------------------------------

static void
gles2_CommandBuffer_SetQueryIndex( Handle cmdBuf, uint32 objectIndex )
{
    CommandBufferPool::Get(cmdBuf)->Command( GLES2__SET_QUERY_INDEX, objectIndex );
}


//------------------------------------------------------------------------------

static void
gles2_CommandBuffer_SetQueryBuffer( Handle cmdBuf, Handle queryBuf )
{
    CommandBufferPool::Get(cmdBuf)->Command( GLES2__SET_QUERY_BUFFER, queryBuf );
}


//------------------------------------------------------------------------------

static void
gles2_CommandBuffer_SetFragmentConstBuffer( Handle cmdBuf, uint32 bufIndex, Handle buffer )
{
//    L_ASSERT(buffer);
    DVASSERT(bufIndex < MAX_CONST_BUFFER_COUNT);
    
    if( buffer != InvalidHandle )
        CommandBufferPool::Get(cmdBuf)->Command( GLES2__SET_FRAGMENT_PROG_CONST_BUFFER, bufIndex, buffer, (uint64)(ConstBufferGLES2::Instance(buffer)) );
}


//------------------------------------------------------------------------------

static void
gles2_CommandBuffer_SetFragmentTexture( Handle cmdBuf, uint32 unitIndex, Handle tex )
{
//    L_ASSERT(tex);

    if( tex != InvalidHandle )
        CommandBufferPool::Get(cmdBuf)->Command( GLES2__SET_FRAGMENT_TEXTURE, unitIndex, tex );
}


//------------------------------------------------------------------------------

static void
gles2_CommandBuffer_SetDepthStencilState( Handle cmdBuf, Handle depthStencilState )
{
    CommandBufferPool::Get(cmdBuf)->Command( GLES2__SET_DEPTHSTENCIL_STATE, depthStencilState );
}


//------------------------------------------------------------------------------

static void 
gles2_CommandBuffer_SetSamplerState( Handle cmdBuf, const Handle samplerState )
{
    // NOTE: expected to be called BEFORE SetFragmentTexture
    CommandBufferPool::Get(cmdBuf)->Command( GLES2__SET_SAMPLER_STATE, samplerState );
}


//------------------------------------------------------------------------------

static void
gles2_CommandBuffer_DrawPrimitive( Handle cmdBuf, PrimitiveType type, uint32 count )
{
    unsigned    v_cnt   = 0;
    int         mode    = GL_TRIANGLES;

    switch( type )
    {
        case PRIMITIVE_TRIANGLELIST :
            v_cnt = count*3;
            mode  = GL_TRIANGLES;
            break;
        
        case PRIMITIVE_LINELIST :
            v_cnt = count*2;
            mode  = GL_LINES;
            break;
            
        default: {}
    }

    CommandBufferPool::Get(cmdBuf)->Command( GLES2__DRAW_PRIMITIVE, uint32(mode), v_cnt );
}


//------------------------------------------------------------------------------

static void
gles2_CommandBuffer_DrawIndexedPrimitive( Handle cmdBuf, PrimitiveType type, uint32 count, uint32 /*vertexCount*/, uint32 firstVertex, uint32 startIndex )
{
    unsigned    v_cnt   = 0;
    int         mode    = GL_TRIANGLES;

    switch( type )
    {
        case PRIMITIVE_TRIANGLELIST :
            v_cnt = count*3;
            mode  = GL_TRIANGLES;
            break;
        
        case PRIMITIVE_LINELIST :
            v_cnt = count*2;
            mode  = GL_LINES;
            break;
        
        default: {}
    }

    CommandBufferPool::Get(cmdBuf)->Command( GLES2__DRAW_INDEXED_PRIMITIVE, uint32(mode), v_cnt, firstVertex, startIndex );
}



//------------------------------------------------------------------------------

static void
gles2_CommandBuffer_SetMarker( Handle cmdBuf, const char* text )
{
/*
    CommandBufferGLES2_t*   cb = CommandBufferPool::Get(cmdBuf);

    if( !cb->text )
    {
        cb->text = new RingBuffer();
        cb->text->Initialize( 64*1024 );
    }
    
    int     len = strlen( text );
    char*   txt = (char*)cb->text->Alloc( len/sizeof(float)+1 );

    memcpy( txt, text, len );
    txt[len] = '\0';

    cb->Command( GLES2__SET_MARKER, (uint64)(txt) );
*/
}






CommandBufferGLES2_t::CommandBufferGLES2_t()
  : isFirstInPass(true),
    isLastInPass(true)
{
}


//------------------------------------------------------------------------------


CommandBufferGLES2_t::~CommandBufferGLES2_t()
{
}


//------------------------------------------------------------------------------

void
CommandBufferGLES2_t::Begin()
{
    _cmd.clear();
}


//------------------------------------------------------------------------------

void        
CommandBufferGLES2_t::End()
{
    _cmd.push_back( EndCmd );
}


//------------------------------------------------------------------------------

void
CommandBufferGLES2_t::Command( uint64 cmd )
{
    _cmd.push_back( cmd );
}


//------------------------------------------------------------------------------

void
CommandBufferGLES2_t::Command( uint64 cmd, uint64 arg1 )
{
    _cmd.resize( _cmd.size()+1+1 );

    std::vector<uint64>::iterator   b = _cmd.end() - (1+1);

    b[0] = cmd; 
    b[1] = arg1; 
}


//------------------------------------------------------------------------------

void
CommandBufferGLES2_t::Command( uint64 cmd, uint64 arg1, uint64 arg2 )
{
    _cmd.resize( _cmd.size()+1+2 );

    std::vector<uint64>::iterator   b = _cmd.end() - (1+2);

    b[0] = cmd; 
    b[1] = arg1; 
    b[2] = arg2; 
}


//------------------------------------------------------------------------------

void
CommandBufferGLES2_t::Command( uint64 cmd, uint64 arg1, uint64 arg2, uint64 arg3 )
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
CommandBufferGLES2_t::Command( uint64 cmd, uint64 arg1, uint64 arg2, uint64 arg3, uint64 arg4 )
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
CommandBufferGLES2_t::Command( uint64 cmd, uint64 arg1, uint64 arg2, uint64 arg3, uint64 arg4, uint64 arg5 )
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
CommandBufferGLES2_t::Command( uint64 cmd, uint64 arg1, uint64 arg2, uint64 arg3, uint64 arg4, uint64 arg5, uint64 arg6 )
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
CommandBufferGLES2_t::Execute()
{
SCOPED_NAMED_TIMING("gl.cb-exec");
    Handle      cur_ps          = InvalidHandle;
    uint32      cur_vdecl       = VertexLayout::InvalidUID;
    Handle      last_ps         = InvalidHandle;
    Handle      vp_const[MAX_CONST_BUFFER_COUNT];
    const void* vp_const_data[MAX_CONST_BUFFER_COUNT];
    Handle      fp_const[MAX_CONST_BUFFER_COUNT];
    const void* fp_const_data[MAX_CONST_BUFFER_COUNT];
    Handle      cur_vb          = InvalidHandle;
    unsigned    tex_unit_0      =0;
    Handle      cur_query_buf   = InvalidHandle;
    uint32      cur_query_i     = InvalidIndex;
    GLint       def_viewport[4];

    for( unsigned i=0; i!=MAX_CONST_BUFFER_COUNT; ++i )
    {
        vp_const[i] = InvalidHandle;
        fp_const[i] = InvalidHandle;
    }
    memset( vp_const_data, 0, sizeof(vp_const_data) );
    memset( fp_const_data, 0, sizeof(fp_const_data) );

    int immediate_cmd_ttw = 10;

    for( std::vector<uint64>::const_iterator c=_cmd.begin(),c_end=_cmd.end(); c!=c_end; ++c )
    {
        const uint64                        cmd = *c;
        std::vector<uint64>::const_iterator arg = c+1;

        if( cmd == EndCmd )
            break;

        switch( cmd )
        {
            case GLES2__BEGIN :
            {
                GL_CALL(glFrontFace( GL_CW ));
                GL_CALL(glEnable( GL_CULL_FACE ));
                GL_CALL(glCullFace( GL_BACK ));
                
                GL_CALL(glEnable( GL_DEPTH_TEST ));
                GL_CALL(glDepthFunc( GL_LEQUAL ));
                GL_CALL(glDepthMask( GL_TRUE ));

                if( isFirstInPass )
                {
#if defined(__DAVAENGINE_IPHONE__)
                    ios_gl_begin_frame();
#endif
                    GLuint  flags = 0;

                    if( passCfg.colorBuffer[0].texture != InvalidHandle )
                    {
                        TextureGLES2::SetAsRenderTarget( passCfg.colorBuffer[0].texture );
                    }
                
                    if( passCfg.colorBuffer[0].loadAction == LOADACTION_CLEAR )
                    {
                        glClearColor( passCfg.colorBuffer[0].clearColor[0], passCfg.colorBuffer[0].clearColor[1], passCfg.colorBuffer[0].clearColor[2], passCfg.colorBuffer[0].clearColor[3] );
                        flags |= GL_COLOR_BUFFER_BIT;
                    }

                    if( passCfg.depthStencilBuffer.loadAction == LOADACTION_CLEAR )
                    {
                        #if defined(__DAVAENGINE_IPHONE__)
                        glClearDepthf( passCfg.depthStencilBuffer.clearDepth );
                        #else
                        glClearDepth( passCfg.depthStencilBuffer.clearDepth );
                        glClearStencil( 0 );
                        #endif

                        flags |= GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT;
                    }

                    if(flags)
                    {
                        glClear( flags );
                    }
                }
                
                glGetIntegerv( GL_VIEWPORT, def_viewport );
            }   break;
            
            case GLES2__END :
            {
                if( isLastInPass )
                {
                    glFlush();

                    if (_GLES2_Binded_FrameBuffer != _GLES2_Default_FrameBuffer)
                    {
                        glBindFramebuffer( GL_FRAMEBUFFER, _GLES2_Default_FrameBuffer );
                        _GLES2_Binded_FrameBuffer = _GLES2_Default_FrameBuffer;
                    }
                }
            }   break;
            
            case GLES2__SET_VERTEX_DATA :
            {
                Handle  vb = (Handle)(arg[0]);

                if( cur_vb != vb )
                {
                    VertexBufferGLES2::SetToRHI( vb );
                    PipelineStateGLES2::SetVertexDeclToRHI( cur_ps, cur_vdecl );
                    cur_vb = vb;
                }
                
                c += 2;
            }   break;
            
            case GLES2__SET_INDICES :
            {
                IndexBufferGLES2::SetToRHI( (Handle)(arg[0]) );
                c += 1;
            }   break;

            case GLES2__SET_QUERY_BUFFER :
            {
                DVASSERT(cur_query_buf == InvalidHandle);
                cur_query_buf = (Handle)(arg[0]);
                c += 1;
            }   break;

            case GLES2__SET_QUERY_INDEX :
            {
                cur_query_i = uint32(arg[0]);
                c += 1;
            }   break;

            case GLES2__SET_PIPELINE_STATE :
            {
                Handle  ps    = (Handle)arg[0];
                uint32  vdecl = (uint32)(arg[1]);
                
                if( cur_ps != ps  ||  cur_vdecl != vdecl )
                {
                    for( unsigned i=0; i!=MAX_CONST_BUFFER_COUNT; ++i )
                    {
                        vp_const[i] = InvalidHandle;
                        fp_const[i] = InvalidHandle;
                    }
                    memset( vp_const_data, 0, sizeof(vp_const_data) );
                    memset( fp_const_data, 0, sizeof(fp_const_data) );

                    cur_ps    = ps;
                    cur_vdecl = vdecl;
                    last_ps   = InvalidHandle;
                    cur_vb    = InvalidHandle;
                }

                tex_unit_0 = PipelineStateGLES2::VertexSamplerCount( ps );

                c += 2;
            }   break;
            
            case GLES2__SET_CULL_MODE :
            {
                switch( CullMode(arg[0]) )
                {
                    case CULL_NONE :
                        glDisable( GL_CULL_FACE );
                        break;

                    case CULL_CCW :
                        glEnable( GL_CULL_FACE );
                        glFrontFace( GL_CW );
                        glCullFace( GL_BACK );
                        break;

                    case CULL_CW :
                        glEnable( GL_CULL_FACE );
                        glFrontFace( GL_CW );
                        glCullFace( GL_FRONT );
                        break;
                }

                c += 1;
            }   break;
            
            case GLES2__SET_SCISSOR_RECT :
            {               
                GLint   x = GLint(arg[0]);
                GLint   y = GLint(arg[1]);
                GLsizei w = GLsizei(arg[2]);
                GLsizei h = GLsizei(arg[3]);

                if( x  &&  y  &&  w  &&  h )
                {
                    glEnable( GL_SCISSOR_TEST );
                    glScissor( x, y, w, h );
                }
                else
                {
                    glDisable( GL_SCISSOR_TEST );
                }

                c += 4;
            }   break;

            case GLES2__SET_VIEWPORT :
            {
                GLint   x = GLint(arg[0]);
                GLint   y = GLint(arg[1]);
                GLsizei w = GLsizei(arg[2]);
                GLsizei h = GLsizei(arg[3]);

                if( !(x==0  &&  y==0  &&  w==0  &&  h==0) )
                {
                    glViewport( x, y, w, h );
                }
                else
                {
                    glViewport( def_viewport[0], def_viewport[1], def_viewport[2], def_viewport[3] );
                }

                c += 4;
            }    break;

            case GLES2__SET_DEPTHSTENCIL_STATE :
            {
                DepthStencilStateGLES2::SetToRHI( (Handle)(arg[0]) );
                c += 1;
            }   break;
            
            case GLES2__SET_SAMPLER_STATE :
            {
                SamplerStateGLES2::SetToRHI( (Handle)(arg[0]) );
                c += 1;
            }   break;

            case GLES2__SET_VERTEX_PROG_CONST_BUFFER :
            {
                unsigned    buf_i = (unsigned)(arg[0]);
                const void* inst  = (const void*)arg[2];

                if( inst != vp_const_data[buf_i] )
                {
                    vp_const[buf_i]      = (Handle)(arg[1]);
                    vp_const_data[buf_i] = inst;
                }
                else
                {
                    vp_const[buf_i] = InvalidHandle;
                }
                
                c += 3;
            }   break;

            case GLES2__SET_FRAGMENT_PROG_CONST_BUFFER :
            {
                unsigned    buf_i = (unsigned)(arg[0]);
                const void* inst  = (const void*)arg[2];

                if( inst != fp_const_data[buf_i] )
                {
                    fp_const[buf_i]      = (Handle)(arg[1]);
                    fp_const_data[buf_i] = inst;
                }
                else
                {
                    fp_const[buf_i] = InvalidHandle;
                }
                
                c += 3;
            }   break;

            case GLES2__SET_FRAGMENT_TEXTURE :
            {
                TextureGLES2::SetToRHI( (Handle)(arg[1]), unsigned(arg[0]), tex_unit_0 );
                StatSet::IncStat( stat_SET_TEX, 1 );
                c += 2;
            }   break;
            
            case GLES2__SET_VERTEX_TEXTURE :
            {
                TextureGLES2::SetToRHI( (Handle)(arg[1]), unsigned(arg[0]), InvalidIndex );
                StatSet::IncStat( stat_SET_TEX, 1 );
                c += 2;
            }   break;

            case GLES2__DRAW_PRIMITIVE :
            {
                unsigned    v_cnt   = unsigned(arg[1]);
                int         mode    = int(arg[0]);
                
                if( last_ps != cur_ps )
                {
                    PipelineStateGLES2::SetToRHI( cur_ps, cur_vdecl );
                    StatSet::IncStat( stat_SET_PS, 1 );
                    last_ps = cur_ps;
                }

                for( unsigned i=0; i!=MAX_CONST_BUFFER_COUNT; ++i )
                {
                    if( vp_const[i] != InvalidHandle )
                        ConstBufferGLES2::SetToRHI( vp_const[i], vp_const_data[i] );
                }
                for( unsigned i=0; i!=MAX_CONST_BUFFER_COUNT; ++i )
                {
                    if( fp_const[i] != InvalidHandle )
                        ConstBufferGLES2::SetToRHI( fp_const[i], fp_const_data[i] );
                }

                if( cur_query_i != InvalidIndex )
                    QueryBufferGLES2::BeginQuery( cur_query_buf, cur_query_i );
                
                GL_CALL(glDrawArrays( mode, 0, v_cnt ));
                StatSet::IncStat( stat_DP, 1 );
//Logger::Info( "  dp" );
                
                if( cur_query_i != InvalidIndex )
                    QueryBufferGLES2::EndQuery( cur_query_buf, cur_query_i );

                c += 2;
            }   break;
            
            case GLES2__DRAW_INDEXED_PRIMITIVE :
            {
                unsigned    v_cnt       = unsigned(arg[1]);
                int         mode        = int(arg[0]);
                uint32      firstVertex = uint32(arg[2]);
                uint32      startIndex  = uint32(arg[3]);

                if( last_ps != cur_ps )
                {
                    PipelineStateGLES2::SetToRHI( cur_ps, cur_vdecl );
                    StatSet::IncStat( stat_SET_PS, 1 );
                    last_ps = cur_ps;
                }

                for( unsigned i=0; i!=MAX_CONST_BUFFER_COUNT; ++i )
                {
                    if( vp_const[i] != InvalidHandle )
                        ConstBufferGLES2::SetToRHI( vp_const[i], vp_const_data[i] );
                }
                for( unsigned i=0; i!=MAX_CONST_BUFFER_COUNT; ++i )
                {
                    if( fp_const[i] != InvalidHandle )
                        ConstBufferGLES2::SetToRHI( fp_const[i], fp_const_data[i] );
                }

                if( firstVertex )
                {
                    PipelineStateGLES2::SetVertexDeclToRHI( cur_ps, cur_vdecl, firstVertex );
                }

                if( cur_query_i != InvalidIndex )
                    QueryBufferGLES2::BeginQuery( cur_query_buf, cur_query_i );

                GL_CALL(glDrawElements( mode, v_cnt, GL_UNSIGNED_SHORT, (void*)(startIndex*sizeof(uint16)) ));
                StatSet::IncStat( stat_DIP, 1 );
//Logger::Info( "  dip" );

                if( cur_query_i != InvalidIndex )
                    QueryBufferGLES2::EndQuery( cur_query_buf, cur_query_i );

                c += 4;
            }   break;

            case GLES2__SET_MARKER :
            {
                Logger::Info( (const char*)(arg[0]) );
                c += 1;
            }   break;

        }
        
        if( --immediate_cmd_ttw )
        {
            _PendingImmediateCmdSync.Lock();
            if( _PendingImmediateCmd )
            {
                _ExecGL( _PendingImmediateCmd, _PendingImmediateCmdCount );
                _PendingImmediateCmd      = nullptr;
                _PendingImmediateCmdCount = 0;
            }
            _PendingImmediateCmdSync.Unlock();

            immediate_cmd_ttw = 10;
        }
    }

    _cmd.clear();
}


//------------------------------------------------------------------------------

static void
_ExecuteQueuedCommands()
{
    static std::vector<uint32>  queue_i;
    
    _CmdQueueSync.Lock();
    _CurRenderQueueSize = _RenderQueue.size();
    queue_i.clear();
    queue_i.swap( _RenderQueue );
    _CmdQueueSync.Unlock();

    _CmdBufIsBeingExecutedSync.Lock();
    _CmdBufIsBeingExecuted = true;
    _CmdBufIsBeingExecutedSync.Unlock();

    for( unsigned i=0; i!=queue_i.size(); ++i )
    {
        std::vector<Handle>*                    cmd_queue = _CmdQueue + queue_i[i];
        static std::vector<RenderPassGLES2_t*>  pass;


        // sort cmd-lists by priority

        pass.clear();
        for( unsigned i=0; i!=cmd_queue->size(); ++i )
        {
            RenderPassGLES2_t*  rp     = RenderPassPool::Get( (*cmd_queue)[i] );
            bool                do_add = true;
        
            for( std::vector<RenderPassGLES2_t*>::iterator p=pass.begin(),p_end=pass.end(); p!=p_end; ++p )
            {
                if( rp->priority > (*p)->priority )
                {
                    pass.insert( p, 1, rp );
                    do_add = false;
                    break;
                }
            }

            if( do_add )
                pass.push_back( rp );
        }
    
        
        // execute command-lists
        
        for( std::vector<RenderPassGLES2_t*>::iterator p=pass.begin(),p_end=pass.end(); p!=p_end; ++p )
        {
            for( unsigned b=0; b!=(*p)->cmdBuf.size(); ++b )
            {
                Handle                  cb_h = (*p)->cmdBuf[b];
                CommandBufferGLES2_t*   cb   = CommandBufferPool::Get( cb_h );

                cb->Execute();
                CommandBufferPool::Free( cb_h );
            }
        }
    
        for( unsigned i=0; i!=cmd_queue->size(); ++i )
            RenderPassPool::Free( (*cmd_queue)[i] );
        cmd_queue->clear();
    }
    
    _CmdBufIsBeingExecutedSync.Lock();
    _CmdBufIsBeingExecuted = false;
    _CmdBufIsBeingExecutedSync.Unlock();

    _CmdQueueSync.Lock();
    _CurRenderQueueSize = 0;
    _CmdQueueSync.Unlock();

    if (_GLES2_Context)
    {
#if defined(__DAVAENGINE_WIN32__)
        HWND    wnd = (HWND)_GLES2_Native_Window;
        HDC     dc = ::GetDC(wnd);
        SwapBuffers(dc);
#elif defined(__DAVAENGINE_MACOS__)
        macos_gl_end_frame();
#elif defined(__DAVAENGINE_IPHONE__)
        ios_gl_end_frame();
#endif
    }
}


//------------------------------------------------------------------------------

static void
gles2_Present()
{
    uint32  sz = 0;

    // CRAP: busy wait
    do
    {
        _CmdQueueSync.Lock();
        sz = uint32(_RenderQueue.size()) + _CurRenderQueueSize;
        _CmdQueueSync.Unlock();        
    } while( sz >= _CmdQueueCount );

    _CmdQueueSync.Lock();
    _RenderQueue.push_back( _CurCmdQueueIndex );    
    if( ++_CurCmdQueueIndex >= _CmdQueueCount )
        _CurCmdQueueIndex = 0;
    _CmdQueueSync.Unlock();
    
#if USE_RENDER_THREAD
#else
    _ExecuteQueuedCommands(); 
#endif

    ProgGLES2::InvalidateAllConstBufferInstances();
}


//------------------------------------------------------------------------------

static void
_RenderFunc( DAVA::BaseObject* obj, void*, void* )
{
    DVASSERT(_GLES2_Make_Context_Current);

    _GLES2_Make_Context_Current();

    _RenderThredStartedSync.Post();
    Logger::Info( "RHI render-thread started" );

    while( true )
    {
        size_t  cnt     = 0;
        bool    do_exit = false;
        
        // CRAP: busy-wait
        do
        {
            _RenderThreadExitSync.Lock();
            do_exit = _RenderThreadExitPending;
            _RenderThreadExitSync.Unlock();
            
            if( do_exit )
                break;

            
            _PendingImmediateCmdSync.Lock();
            if( _PendingImmediateCmd )
            {
                _ExecGL( _PendingImmediateCmd, _PendingImmediateCmdCount );
                _PendingImmediateCmd      = nullptr;
                _PendingImmediateCmdCount = 0;
            }
            _PendingImmediateCmdSync.Unlock();


            _CmdQueueSync.Lock();
            cnt = _RenderQueue.size();
            _CmdQueueSync.Unlock();
        } while( cnt == 0 );

        if( do_exit )
            break;

        _ExecuteQueuedCommands();                 
    }

    Logger::Info( "RHI render-thread stopped" );
}

void
InitializeRenderThread()
{
#if USE_RENDER_THREAD
    RenderThread = DAVA::Thread::Create( DAVA::Message(&_RenderFunc) );
    RenderThread->Start();    
    _RenderThredStartedSync.Wait();
#endif
}


//------------------------------------------------------------------------------

void
UninitializeRenderThread()
{
#if USE_RENDER_THREAD
    _RenderThreadExitSync.Lock();
    _RenderThreadExitPending = true;
    _RenderThreadExitSync.Unlock();

    RenderThread->Join();
#endif
}


//------------------------------------------------------------------------------

static void
_ExecGL( GLCommand* command, uint32 cmdCount )
{
    int     err = 0;

#if 0

    #define EXEC_GL(expr) \
    expr ; \
    err = glGetError(); \
    if( err != GL_NO_ERROR ) \
        Logger::Error( "FAILED  %s (%i) : %s\n", #expr, err, GetGLErrorString(err) );

#else

    #define EXEC_GL(expr) expr 

#endif

    for( GLCommand* cmd=command,*cmdEnd=command+cmdCount; cmd!=cmdEnd; ++cmd )
    {
        const uint64*   arg = cmd->arg;

        switch( cmd->func )
        {
            case GLCommand::NOP :
            {
                // do NOTHING
            }   break;
            
            case GLCommand::GEN_BUFFERS :
            {
                EXEC_GL(glGenBuffers( (GLsizei)(arg[0]), (GLuint*)(arg[1]) ));
                cmd->status = err;
            }   break;

            case GLCommand::BIND_BUFFER :
            {
                EXEC_GL(glBindBuffer( (GLenum)(arg[0]), (GLuint)(arg[1]) ));
                cmd->status = err;
            }   break;

            case GLCommand::DELETE_BUFFERS :
            {
                EXEC_GL(glDeleteBuffers( (GLsizei)(arg[0]), (GLuint*)(arg[1]) ));
                cmd->status = err;
            }   break;

            case GLCommand::BUFFER_DATA :
            {
                EXEC_GL(glBufferData( (GLenum)(arg[0]), (GLsizei)(arg[1]), (const void*)(arg[2]), (GLenum)(arg[3]) ));
                cmd->status = err;
            }   break;

            case GLCommand::GEN_TEXTURES :
            {
                EXEC_GL(glGenTextures( (GLsizei)(arg[0]), (GLuint*)(arg[1]) ));
                cmd->status = err;
            }   break;
            
            case GLCommand::DELETE_TEXTURES :
            {
                EXEC_GL(glDeleteTextures( (GLsizei)(arg[0]), (GLuint*)(arg[1]) ));
                cmd->status = err;
            }   break;

            case GLCommand::BIND_TEXTURE :
            {
                EXEC_GL(glBindTexture( (GLenum)(cmd->arg[0]), (GLuint)(cmd->arg[1]) ));
                cmd->status = err;
            }   break;

            case GLCommand::TEX_PARAMETER_I :
            {
                EXEC_GL(glTexParameteri( (GLenum)(arg[0]), (GLenum)(arg[1]), (GLuint)(arg[2]) ));
                cmd->status = err;
            }   break;

            case GLCommand::TEX_IMAGE2D :
            {
                if(arg[10])
                {
                    EXEC_GL(glCompressedTexImage2D( (GLenum)(arg[0]), (GLint)(arg[1]), (GLenum)(arg[2]), (GLsizei)(arg[3]), (GLsizei)(arg[4]), (GLint)(arg[5]), (GLsizei)(arg[8]), (const GLvoid*)(arg[9]) ));
                }
                else
                {
                    EXEC_GL(glTexImage2D( (GLenum)(arg[0]), (GLint)(arg[1]), (GLint)(arg[2]), (GLsizei)(arg[3]), (GLsizei)(arg[4]), (GLint)(arg[5]), (GLenum)(arg[6]), (GLenum)(arg[7]), (const GLvoid*)(arg[9]) ));
                }
                cmd->status = err;
            }   break;

            case GLCommand::GENERATE_MIPMAP :
            {
                EXEC_GL(glGenerateMipmap( (GLenum)(arg[0]) ));
                cmd->status = err;
            }   break;

            case GLCommand::READ_PIXELS :
            {
                EXEC_GL(glReadPixels( GLint(arg[0]), GLint(arg[1]), GLsizei(arg[2]), GLsizei(arg[3]), GLenum(arg[4]), GLenum(arg[5]), (GLvoid*)(arg[6]) ));
                cmd->retval = 0;
                cmd->status = err;
            }   break;

            case GLCommand::CREATE_SHADER :
            {
                cmd->retval = glCreateShader( (GLenum)(arg[0]) );
                cmd->status = 0;
            }   break;

            case GLCommand::SHADER_SOURCE :
            {
                EXEC_GL(glShaderSource( (GLuint)(arg[0]), (GLsizei)(arg[1]), (const GLchar**)(arg[2]), (const GLint*)(arg[3]) ));
                cmd->status = err;
            }   break;

            case GLCommand::COMPILE_SHADER :
            {
                EXEC_GL(glCompileShader( (GLuint)(arg[0]) ));
                cmd->status = err;
            }   break;

            case GLCommand::GET_SHADER_IV :
            {
                EXEC_GL(glGetShaderiv( (GLuint)(arg[0]), (GLenum)(arg[1]), (GLint*)(arg[2]) ));
                cmd->status = err;
            }   break;

            case GLCommand::GET_SHADER_INFO_LOG :
            {
                EXEC_GL(glGetShaderInfoLog( (GLuint)(arg[0]), GLsizei(arg[1]), (GLsizei*)(arg[2]), (GLchar*)(arg[3]) ));
                cmd->status = err;
            }   break;

            case GLCommand::GET_PROGRAM_IV :
            {
                EXEC_GL(glGetProgramiv( (GLuint)(arg[0]), (GLenum)(arg[1]), (GLint*)(arg[2]) ));
                cmd->status = err;
            }   break;

            case GLCommand::GET_ACTIVE_UNIFORM :
            {
                EXEC_GL(glGetActiveUniform( (GLuint)(arg[0]), (GLuint)(arg[1]), (GLsizei)(arg[2]), (GLsizei*)(arg[3]), (GLint*)(arg[4]), (GLenum*)(arg[5]), (GLchar*)(arg[6]) ));
                cmd->status = err;
            }   break;

            case GLCommand::GET_UNIFORM_LOCATION :
            {
                cmd->retval = glGetUniformLocation( (GLuint)(arg[0]), (const GLchar*)(arg[1]) );
                cmd->status = 0;
            }   break;        

            case GLCommand::SET_UNIFORM_1I :
            {
                EXEC_GL(glUniform1i( GLint(arg[0]), GLint(arg[1]) ));
            }   break;

            case GLCommand::GEN_FRAMEBUFFERS :
            {
                EXEC_GL(glGenFramebuffers( (GLuint)(arg[0]), (GLuint*)(arg[1]) ));
                cmd->status = err;
            }   break;        

            case GLCommand::GEN_RENDERBUFFERS :
            {
                EXEC_GL(glGenRenderbuffers( (GLuint)(arg[0]), (GLuint*)(arg[1]) ));
                cmd->status = err;
            }   break;        

            case GLCommand::BIND_FRAMEBUFFER :
            {
                EXEC_GL(glBindFramebuffer( (GLenum)(arg[0]), (GLuint)(arg[1]) ));
                cmd->status = err;
            }   break;        

            case GLCommand::BIND_RENDERBUFFER :
            {
                EXEC_GL(glBindRenderbuffer( (GLenum)(arg[0]), (GLuint)(arg[1]) ));
                cmd->status = err;
            }   break;        

            case GLCommand::FRAMEBUFFER_TEXTURE :
            {
                EXEC_GL(glFramebufferTexture2D( GLenum(arg[0]), GLenum(arg[1]), GLenum(arg[2]), GLuint(arg[3]), GLint(arg[4]) ));
                cmd->status = err;
            }   break;        

            case GLCommand::RENDERBUFFER_STORAGE :
            {
                EXEC_GL(glRenderbufferStorage( GLenum(arg[0]), GLenum(arg[1]), GLsizei(arg[2]), GLsizei(arg[3]) ));
                cmd->status = err;
            }   break;

            case GLCommand::FRAMEBUFFER_RENDERBUFFER :
            {
                EXEC_GL(glFramebufferRenderbuffer( GLenum(arg[0]), GLenum(arg[1]), GLenum(arg[2]), GLuint(arg[3]) ));
                cmd->status = err;
            }   break;

            case GLCommand::FRAMEBUFFER_STATUS :
            {
                cmd->retval = glCheckFramebufferStatus( GLenum(arg[0]) );
                cmd->status = 0;
            }   break;        

            case GLCommand::DELETE_FRAMEBUFFERS :
            {
                EXEC_GL(glDeleteFramebuffers( GLsizei(arg[0]), (const GLuint*)(arg[1]) ));
                cmd->retval = 0;
                cmd->status = err;
            }   break;

            case GLCommand::DRAWBUFFERS :
            {
                #if defined __DAVAENGINE_IPHONE__
                #else
                EXEC_GL(glDrawBuffers( GLuint(arg[0]), (GLenum*)(arg[1]) ));
                cmd->status = err;
                #endif
            }   break;

        }
    }
#undef EXEC_GL
}
    
    
//------------------------------------------------------------------------------

void
ExecGL( GLCommand* command, uint32 cmdCount )
{
#if USE_RENDER_THREAD

    bool    scheduled = false;
    bool    executed  = false;
    
    // CRAP: busy-wait
    do
    {
        _PendingImmediateCmdSync.Lock();
        if( !_PendingImmediateCmd )
        {
            _PendingImmediateCmd      = command;
            _PendingImmediateCmdCount = cmdCount;
            scheduled                 = true;
        }
        _PendingImmediateCmdSync.Unlock();
    } while( !scheduled );

    // CRAP: busy-wait
    do
    {
        _PendingImmediateCmdSync.Lock();
        if( !_PendingImmediateCmd )
        {
            executed = true;
        }
        _PendingImmediateCmdSync.Unlock();
    } while( !executed );

#else

    _ExecGL( command, cmdCount );

#endif
}



namespace CommandBufferGLES2
{
void
SetupDispatch( Dispatch* dispatch )
{
    dispatch->impl_CommandBuffer_Begin                  = &gles2_CommandBuffer_Begin;
    dispatch->impl_CommandBuffer_End                    = &gles2_CommandBuffer_End;
    dispatch->impl_CommandBuffer_SetPipelineState       = &gles2_CommandBuffer_SetPipelineState;
    dispatch->impl_CommandBuffer_SetCullMode            = &gles2_CommandBuffer_SetCullMode;
    dispatch->impl_CommandBuffer_SetScissorRect         = &gles2_CommandBuffer_SetScissorRect;
    dispatch->impl_CommandBuffer_SetViewport            = &gles2_CommandBuffer_SetViewport;
    dispatch->impl_CommandBuffer_SetVertexData          = &gles2_CommandBuffer_SetVertexData;
    dispatch->impl_CommandBuffer_SetVertexConstBuffer   = &gles2_CommandBuffer_SetVertexConstBuffer;
    dispatch->impl_CommandBuffer_SetVertexTexture       = &gles2_CommandBuffer_SetVertexTexture;
    dispatch->impl_CommandBuffer_SetIndices             = &gles2_CommandBuffer_SetIndices;
    dispatch->impl_CommandBuffer_SetQueryBuffer         = &gles2_CommandBuffer_SetQueryBuffer;
    dispatch->impl_CommandBuffer_SetQueryIndex          = &gles2_CommandBuffer_SetQueryIndex;
    dispatch->impl_CommandBuffer_SetFragmentConstBuffer = &gles2_CommandBuffer_SetFragmentConstBuffer;
    dispatch->impl_CommandBuffer_SetFragmentTexture     = &gles2_CommandBuffer_SetFragmentTexture;
    dispatch->impl_CommandBuffer_SetDepthStencilState   = &gles2_CommandBuffer_SetDepthStencilState;
    dispatch->impl_CommandBuffer_SetSamplerState        = &gles2_CommandBuffer_SetSamplerState;
    dispatch->impl_CommandBuffer_DrawPrimitive          = &gles2_CommandBuffer_DrawPrimitive;
    dispatch->impl_CommandBuffer_DrawIndexedPrimitive   = &gles2_CommandBuffer_DrawIndexedPrimitive;
    dispatch->impl_CommandBuffer_SetMarker              = &gles2_CommandBuffer_SetMarker;
    
    dispatch->impl_Present                              = &gles2_Present;
}
}



} // namespace rhi