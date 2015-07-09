﻿
    #include "../Common/rhi_Pool.h"
    #include "rhi_GLES2.h"
    #include "rhi_ProgGLES2.h"

    #include "../Common/rhi_Private.h"
    #include "../Common/dbg_StatSet.h"

    #include "Debug/DVAssert.h"
    #include "FileSystem/Logger.h"
    using DAVA::Logger;
    #include "Concurrency/Thread.h"
    #include "Concurrency/Semaphore.h"
    #include "Debug/Profiler.h"

    #include "_gl.h"

    #define USE_RENDER_THREAD               0
    #define RHI_MAX_PREPARED_FRAME_COUNT    1


namespace rhi
{

enum
CommandGLES2
{
    GLES2__BEGIN                            = 1,
    GLES2__END                              = 2,

    GLES2__SET_VERTEX_DATA                  = 11,
    GLES2__SET_INDICES                      = 12,
    GLES2__SET_QUERY_BUFFER                 = 13,
    GLES2__SET_QUERY_INDEX                  = 14,

    GLES2__SET_PIPELINE_STATE               = 21,
    GLES2__SET_DEPTHSTENCIL_STATE           = 22,
    GLES2__SET_SAMPLER_STATE                = 23,
    GLES2__SET_CULL_MODE                    = 24,
    GLES2__SET_SCISSOR_RECT                 = 25,
    GLES2__SET_VIEWPORT                     = 26,

    GLES2__SET_VERTEX_PROG_CONST_BUFFER     = 31,
    GLES2__SET_FRAGMENT_PROG_CONST_BUFFER   = 32,
    GLES2__SET_VERTEX_TEXTURE               = 33,
    GLES2__SET_FRAGMENT_TEXTURE             = 34,


    GLES2__DRAW_PRIMITIVE                   = 41,
    GLES2__DRAW_INDEXED_PRIMITIVE           = 42,

    GLES2__SET_MARKER                       = 51,


    GLES2__NOP                              = 77
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
    uint32              usingDefaultFrameBuffer:1;

    uint32              dbgCommandCount;
};

typedef Pool<CommandBufferGLES2_t,RESOURCE_COMMAND_BUFFER>  CommandBufferPool;
typedef Pool<RenderPassGLES2_t,RESOURCE_RENDER_PASS>        RenderPassPool;

RHI_IMPL_POOL(CommandBufferGLES2_t,RESOURCE_COMMAND_BUFFER);
RHI_IMPL_POOL(RenderPassGLES2_t,RESOURCE_RENDER_PASS);
    
const uint64   CommandBufferGLES2_t::EndCmd = 0xFFFFFFFF;

static bool                 _GLES2_CmdBufIsBeingExecuted    = false;
static DAVA::Spinlock       _GLES2_CmdBufIsBeingExecutedSync;

static GLCommand*           _GLES2_PendingImmediateCmd      = nullptr;
static uint32               _GLES2_PendingImmediateCmdCount = 0;
static DAVA::Mutex          _GLES2_PendingImmediateCmdSync;

static bool                 _GLES2_RenderThreadExitPending  = false;
static DAVA::Spinlock       _GLES2_RenderThreadExitSync;
static DAVA::Semaphore      _GLES2_RenderThredStartedSync   (1);

#if USE_RENDER_THREAD 
static DAVA::Thread*        _GLES2_RenderThread             = nullptr;
#endif

struct
Frame
{
    unsigned            number;
    std::vector<Handle> pass;
    uint32              readyToExecute:1;
};

static std::vector<Frame>   _Frame;
static bool                 _FrameStarted   = false;
static unsigned             _FrameNumber    = 1;
//static DAVA::Spinlock       _FrameSync;
static DAVA::Mutex          _FrameSync;


static void _ExecGL( GLCommand* command, uint32 cmdCount );



static Handle
gles2_RenderPass_Allocate( const RenderPassConfig& passConf, uint32 cmdBufCount, Handle* cmdBuf )
{
Trace("gl.alloc-rpass\n");
    DVASSERT(cmdBufCount);

    Handle              handle  = RenderPassPool::Alloc();
    RenderPassGLES2_t*  pass    = RenderPassPool::Get( handle );

    pass->cmdBuf.resize( cmdBufCount );
    pass->priority = passConf.priority;

    for( unsigned i=0; i!=cmdBufCount; ++i )
    {
        Handle                  h  = CommandBufferPool::Alloc();
        CommandBufferGLES2_t*   cb = CommandBufferPool::Get( h );
        
        cb->_cmd.clear();
        cb->passCfg                 = passConf;
        cb->isFirstInPass           = i == 0;
        cb->isLastInPass            = i == cmdBufCount - 1;
        cb->usingDefaultFrameBuffer = passConf.colorBuffer[0].texture == InvalidHandle;
        
        pass->cmdBuf[i] = h;
        cmdBuf[i]       = h;
    }

    return handle;
}

static void
gles2_RenderPass_Begin( Handle pass )
{
    _FrameSync.Lock();

    if( !_FrameStarted )
    {
        _Frame.push_back( Frame() );
        _Frame.back().number         = _FrameNumber;
        _Frame.back().readyToExecute = false;

Trace("\n\n-------------------------------\nframe %u started\n",_FrameNumber);
        _FrameStarted = true;
        ++_FrameNumber;
    }

    _Frame.back().pass.push_back( pass );

    _FrameSync.Unlock();
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
gles2_CommandBuffer_Begin( Handle cmdBuf )
{
    CommandBufferPool::Get(cmdBuf)->Begin();
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
dbgCommandCount = 0;
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
++dbgCommandCount;
}


//------------------------------------------------------------------------------

void
CommandBufferGLES2_t::Command( uint64 cmd, uint64 arg1 )
{
    _cmd.resize( _cmd.size()+1+1 );

    std::vector<uint64>::iterator   b = _cmd.end() - (1+1);

    b[0] = cmd; 
    b[1] = arg1; 
++dbgCommandCount;
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
++dbgCommandCount;
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
++dbgCommandCount;
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
++dbgCommandCount;
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
++dbgCommandCount;
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
++dbgCommandCount;
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
    GLint       def_viewport[4] = {0,0,0,0};

    for( unsigned i=0; i!=MAX_CONST_BUFFER_COUNT; ++i )
    {
        vp_const[i] = InvalidHandle;
        fp_const[i] = InvalidHandle;
    }
    memset( vp_const_data, 0, sizeof(vp_const_data) );
    memset( fp_const_data, 0, sizeof(fp_const_data) );

    int immediate_cmd_ttw = 10;

Trace("cmd-count= %i\n",int(dbgCommandCount));
unsigned cmd_n=0;
    for( std::vector<uint64>::const_iterator c=_cmd.begin(),c_end=_cmd.end(); c!=c_end; ++c )
    {
        const uint64                        cmd = *c;
        std::vector<uint64>::const_iterator arg = c+1;

        if( cmd == EndCmd )
            break;

Trace("cmd[%u] %i\n",cmd_n,int(cmd));
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

                    def_viewport[0] = 0;
                    def_viewport[1] = 0;

                    if( passCfg.colorBuffer[0].texture != InvalidHandle )
                    {
                        Size2i  sz = TextureGLES2::Size( passCfg.colorBuffer[0].texture );
                        
                        TextureGLES2::SetAsRenderTarget( passCfg.colorBuffer[0].texture );
                        def_viewport[2] = sz.dx;
                        def_viewport[3] = sz.dy;
                    }
                    else
                    {
                        GLint   fbo = 0;

                        glGetIntegerv( GL_FRAMEBUFFER_BINDING, &fbo );
    
                        if( fbo )
                        {
                            GLint   type = 0;
                            GLint   obj  = 0;

                            glGetFramebufferAttachmentParameteriv( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE, &type );
                            glGetFramebufferAttachmentParameteriv( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME, &obj );
        
                            if( type == GL_RENDERBUFFER )
                            {
                                glGetRenderbufferParameteriv( GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, def_viewport+2 );
                                glGetRenderbufferParameteriv( GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, def_viewport+3 );
                            }
                            else if( type == GL_TEXTURE )
                            {
//                                GLint   w,h;
//
//                                glGetTexParameteriv( GL_TEXTURE_2D, obj,  );
                            }
                        }
                        else
                        {
                            def_viewport[2] = _GLES2_DefaultFrameBuffer_Width;
                            def_viewport[3] = _GLES2_DefaultFrameBuffer_Height;
//                            glGetIntegerv( GL_VIEWPORT, def_viewport );
                        }
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
                    
                    glViewport( def_viewport[0], def_viewport[1], def_viewport[2], def_viewport[3] );
                }
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

                if( !(x==0  &&  y==0  &&  w==0  &&  h==0) )
                {
                    if( usingDefaultFrameBuffer )
                        y = _GLES2_DefaultFrameBuffer_Height - y - h;
                    
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
//LCP;
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
//LCP;

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

//LCP;
Trace("DIP  mode= %i  v_cnt= %i  start_i= %i\n",int(mode),int(v_cnt),int(startIndex));
//LCP;
                GL_CALL(glDrawElements( mode, v_cnt, GL_UNSIGNED_SHORT, (void*)(startIndex*sizeof(uint16)) ));
//LCP;
                StatSet::IncStat( stat_DIP, 1 );
//LCP;

                if( cur_query_i != InvalidIndex )
                    QueryBufferGLES2::EndQuery( cur_query_buf, cur_query_i );

//LCP;
                c += 4;
            }   break;

            case GLES2__SET_MARKER :
            {
                Trace( (const char*)(arg[0]) );
                c += 1;
            }   break;

        }
        
        if( --immediate_cmd_ttw <= 0 )
        {
            _GLES2_PendingImmediateCmdSync.Lock();
            if( _GLES2_PendingImmediateCmd )
            {
                _ExecGL( _GLES2_PendingImmediateCmd, _GLES2_PendingImmediateCmdCount );
                _GLES2_PendingImmediateCmd      = nullptr;
                _GLES2_PendingImmediateCmdCount = 0;
            }
            _GLES2_PendingImmediateCmdSync.Unlock();

            immediate_cmd_ttw = 10;
        }
++cmd_n;    
    }

    _cmd.clear();
}


//------------------------------------------------------------------------------

static void
_ExecuteQueuedCommands()
{
Trace("rhi-gl.exec-queued-cmd\n");

    std::vector<RenderPassGLES2_t*> pass;
    std::vector<Handle>             pass_h;
    unsigned                        frame_n = 0;
    bool                            do_exit = false;

    _FrameSync.Lock();
    if( _Frame.size() )
    {
        for( std::vector<Handle>::iterator p=_Frame.begin()->pass.begin(),p_end=_Frame.begin()->pass.end(); p!=p_end; ++p )
        {
            RenderPassGLES2_t*  pp      = RenderPassPool::Get( *p );
            bool                do_add  = true;
            
            for( unsigned i=0; i!=pass.size(); ++i )
            {
                if( pp->priority > pass[i]->priority )
                {
                    pass.insert( pass.begin()+i, 1, pp );
                    do_add = false;
                    break;
                }
            }
            
            if( do_add )
                pass.push_back( pp );
        }

        pass_h  = _Frame.begin()->pass;
        frame_n = _Frame.begin()->number;
    }
    else
    {
        do_exit = true;
    }
    _FrameSync.Unlock();

    if( do_exit )
        return;

Trace("\n\n-------------------------------\nexecuting frame %u\n",frame_n);
    for( std::vector<RenderPassGLES2_t*>::iterator p=pass.begin(),p_end=pass.end(); p!=p_end; ++p )
    {
        RenderPassGLES2_t*  pp = *p;

        for( unsigned b=0; b!=pp->cmdBuf.size(); ++b )
        {
            Handle                  cb_h = pp->cmdBuf[b];
            CommandBufferGLES2_t*   cb   = CommandBufferPool::Get( cb_h );

            cb->Execute();
            CommandBufferPool::Free( cb_h );
        }
        
//        RenderPassPool::Free( *p );
    }

    _FrameSync.Lock();
    {
Trace("\n\n-------------------------------\nframe %u executed(submitted to GPU)\n",frame_n);
        _Frame.erase( _Frame.begin() );
        
        for( std::vector<Handle>::iterator p=pass_h.begin(),p_end=pass_h.end(); p!=p_end; ++p )
            RenderPassPool::Free( *p );
    }    
    _FrameSync.Unlock();



/*
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
*/

    if( _GLES2_Context )
    {
#if defined(__DAVAENGINE_WIN32__)
        HWND    wnd = (HWND)_GLES2_Native_Window;
        HDC     dc = ::GetDC( wnd );
Trace("rhi-gl.swap-buffers...\n");
        SwapBuffers( dc );
Trace("rhi-gl.swap-buffers done\n");
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
#if USE_RENDER_THREAD

Trace("rhi-gl.present\n");

    _FrameSync.Lock(); 
    {   
        if( _Frame.size() )
        {
            _Frame.back().readyToExecute = true;
            _FrameStarted = false;
Trace("\n\n-------------------------------\nframe %u generated\n",_Frame.back().number);
        }

//        _FrameStarted = false;
    }
    _FrameSync.Unlock();

    unsigned frame_cnt = 0;

    do
    {
    _FrameSync.Lock();
    frame_cnt = _Frame.size();
//Trace("rhi-gl.present frame-cnt= %u\n",frame_cnt);
    _FrameSync.Unlock();
    }
    while( frame_cnt >= RHI_MAX_PREPARED_FRAME_COUNT );

#else

    if( _Frame.size() )
    {
        _Frame.back().readyToExecute = true;
        _FrameStarted = false;
    }

    _ExecuteQueuedCommands(); 

#endif

    ProgGLES2::InvalidateAllConstBufferInstances();
}


//------------------------------------------------------------------------------

static void
_RenderFunc( DAVA::BaseObject* obj, void*, void* )
{
    DVASSERT(_GLES2_AcquireContext);
    _GLES2_AcquireContext();

    _GLES2_RenderThredStartedSync.Post();
    Trace( "RHI render-thread started\n" );

    while( true )
    {
        bool    do_wait = true;
        bool    do_exit = false;
        
        // CRAP: busy-wait
        do
        {
            _GLES2_RenderThreadExitSync.Lock();
            do_exit = _GLES2_RenderThreadExitPending;
            _GLES2_RenderThreadExitSync.Unlock();
            
            if( do_exit )
                break;

            
            _GLES2_PendingImmediateCmdSync.Lock();
            if( _GLES2_PendingImmediateCmd )
            {
//Trace("exec imm cmd (%u)\n",_PendingImmediateCmdCount);
                _ExecGL( _GLES2_PendingImmediateCmd, _GLES2_PendingImmediateCmdCount );
                _GLES2_PendingImmediateCmd      = nullptr;
                _GLES2_PendingImmediateCmdCount = 0;
//Trace("exec-imm-cmd done\n");
            }
            _GLES2_PendingImmediateCmdSync.Unlock();


//            _CmdQueueSync.Lock();
//            cnt = _RenderQueue.size();
//            _CmdQueueSync.Unlock();
            _FrameSync.Lock();
            do_wait = !( _Frame.size()  &&  _Frame.begin()->readyToExecute );
            _FrameSync.Unlock();
        } while( do_wait );

        if( do_exit )
            break;

        _ExecuteQueuedCommands();
    }

    Trace( "RHI render-thread stopped\n" );
}

void
InitializeRenderThread()
{
#if USE_RENDER_THREAD

    DVASSERT(_GLES2_ReleaseContext);
    _GLES2_ReleaseContext();

    RenderThread = DAVA::Thread::Create( DAVA::Message(&_RenderFunc) );
    RenderThread->SetName( "RHI.gl-render" );
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
_LogGLError( const char* expr, int err )
{
    Trace( "FAILED  %s (err= 0x%X) : %s\n", expr, err, GetGLErrorString(err) );
    DVASSERT(!"KABOOM!!!");
}


//------------------------------------------------------------------------------

static void
_ExecGL( GLCommand* command, uint32 cmdCount )
{
    int     err = 0;

    do 
    {
        err = glGetError();
    } 
    while ( err != GL_NO_ERROR );
#if 1

//    while( glGetError() != GL_NO_ERROR )
//        ;

    #define EXEC_GL(expr) \
    expr ; \
    err = glGetError(); \
    if( err != GL_NO_ERROR ) \
        _LogGLError( #expr, err ); \

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

            case GLCommand::CREATE_PROGRAM :
            {
                cmd->retval = glCreateProgram();
                cmd->status = 0;
            }   break;

            case GLCommand::CREATE_SHADER :
            {
                cmd->retval = glCreateShader( (GLenum)(arg[0]) );
                cmd->status = 0;
            }   break;

            case GLCommand::ATTACH_SHADER :
            {
                EXEC_GL(glAttachShader( GLuint(arg[0]), GLuint(arg[1]) ));
                cmd->status = err;
            }   break;

            case GLCommand::LINK_PROGRAM :
            {
                EXEC_GL(glLinkProgram( GLuint(arg[0]) ));
                cmd->status = err;
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

            case GLCommand::GET_ATTRIB_LOCATION :
            {
                cmd->retval = glGetAttribLocation( GLuint(arg[0]), (const GLchar*)(arg[1]) );
                cmd->status = 0;
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
            
            case GLCommand::DELETE_RENDERBUFFERS :
            {
                EXEC_GL(glDeleteRenderbuffers( GLsizei(arg[0]), (const GLuint *)(arg[1]) ));
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
ExecGL( GLCommand* command, uint32 cmdCount, bool force_immediate )
{
#if USE_RENDER_THREAD

    if( force_immediate )
    {
        _ExecGL( command, cmdCount );
    }
    else
    {
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
    }

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