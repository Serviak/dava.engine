
    #include "rhi_ProgGLES2.h"
    #include "../Common/rhi_Private.h"
    #include "../Common/dbg_StatSet.h"
    #include "../Common/rhi_Pool.h"
    #include "../Common/rhi_RingBuffer.h"

    #include "FileSystem/Logger.h"
    using DAVA::Logger;
    #include "Debug/Profiler.h"

    #include "rhi_GLES2.h"
    #include "_gl.h"

    #include <stdio.h>
    #include <string.h>


namespace rhi
{
//==============================================================================

typedef Pool<ProgGLES2::ConstBuf,RESOURCE_CONST_BUFFER>   ConstBufGLES2Pool;
RHI_IMPL_POOL(ProgGLES2::ConstBuf,RESOURCE_CONST_BUFFER);

static RingBuffer   DefaultConstRingBuffer;


//==============================================================================

ProgGLES2::ProgGLES2( ProgType t )
  : type(t),
    shader(0),
    texunitInited(false)
{
    for( unsigned i=0; i!=MAX_CONST_BUFFER_COUNT; ++i )
    {
        cbuf[i].location = InvalidIndex;
        cbuf[i].count    = 0;
    }
}


//------------------------------------------------------------------------------

ProgGLES2::~ProgGLES2()
{
}


//------------------------------------------------------------------------------

bool
ProgGLES2::Construct( const char* srcCode )
{
    bool        success = false;
    int         stype   = (type==PROG_VERTEX) ? GL_VERTEX_SHADER : GL_FRAGMENT_SHADER;
    GLCommand   cmd1    = { GLCommand::CREATE_SHADER, { uint64(stype) } };

    ExecGL( &cmd1, 1 );

    if( cmd1.retval )
    {
        unsigned    s           = cmd1.retval;
        int         status;
        char        info[1024]  = "";
        GLCommand   cmd2[]      =
        {
            { GLCommand::SHADER_SOURCE, { s, 1, uint64(&srcCode), 0 } },
            { GLCommand::COMPILE_SHADER, { s } },
            { GLCommand::GET_SHADER_IV, { s, GL_COMPILE_STATUS, uint64(&status) } },
            { GLCommand::GET_SHADER_INFO_LOG, { s, countof(info), 0, uint64(info) } }
        };

        ExecGL( cmd2, countof(cmd2) );

        if( status )
        {
            shader  = s;
            success = true;            
        }
        else
        {
            Logger::Error( "%sprog-compile failed:", (type==PROG_VERTEX) ? "v" : "f" );
            Logger::Info( info );
        }
    }
    
    return success;
}


//------------------------------------------------------------------------------

void
ProgGLES2::Destroy()
{
}


//------------------------------------------------------------------------------

void
ProgGLES2::GetProgParams( unsigned progUid )
{
#if DV_USE_UNIFORMBUFFER_OBJECT
    for( unsigned i=0; i!=MAX_CONST_BUFFER_COUNT; ++i )
    {
        char    name[32];   sprintf( name, "%s_Buffer%u_Block", (type == PROG_VERTEX)?"VP":"FP", i );
        GLuint  loc         = glGetUniformBlockIndex( progUid, name );

        if( loc != GL_INVALID_INDEX )
        {
            GLint   sz;

            glGetActiveUniformBlockiv( progUid, loc, GL_UNIFORM_BLOCK_DATA_SIZE, &sz );
            GL_CALL(glUniformBlockBinding( progUid, loc, loc ));

            cbuf[i].location = loc;
            cbuf[i].count    = sz / (4*sizeof(float));
        }
        else
        {
            cbuf[i].location = InvalidIndex;
            cbuf[i].count    = 0;
        }
    }
#else

    for( unsigned i=0; i!=MAX_CONST_BUFFER_COUNT; ++i )
    {
        cbuf[i].location = InvalidIndex;
        cbuf[i].count    = 0;
    }

    GLint       cnt  = 0;
    GLCommand   cmd1 = { GLCommand::GET_PROGRAM_IV, { progUid, GL_ACTIVE_UNIFORMS, uint64(&cnt) } };
    
    ExecGL( &cmd1, 1 );

    for( unsigned u=0; u!=cnt; ++u )
    {    
        char        name[64];
        GLsizei     length;
        GLint       size;
        GLenum      utype;
        GLCommand   cmd2    = { GLCommand::GET_ACTIVE_UNIFORM, { progUid, u, uint64(sizeof(name)-1), uint64(&length), uint64(&size), uint64(&utype), uint64(name) } };
        
        ExecGL( &cmd2, 1 );
        
        for( unsigned i=0; i!=MAX_CONST_BUFFER_COUNT; ++i )
        {
            char    n[16];   sprintf( n, "%s_Buffer%u[0]", (type == PROG_VERTEX)?"VP":"FP", i );
            
            if( !strcmp( name, n ) )
            {            
                int         loc;
                GLCommand   cmd3 = { GLCommand::GET_UNIFORM_LOCATION, { progUid, uint64(name) } };

                ExecGL( &cmd3, 1 );
                loc = cmd3.retval;

                if( loc != -1 )
                {
                    cbuf[i].location = loc;
                    cbuf[i].count    = size;
                    break;
                }
            }
        }
    }
#endif // DV_USE_UNIFORMBUFFER_OBJECT


    // get texture location
    {
        char        tname[countof(texunitLoc)][16];
        GLCommand   cmd[countof(texunitLoc)];

        for( unsigned i=0; i!=countof(texunitLoc); ++i )
        {
            char    name[16];   

            Snprinf( name, countof(name), "Texture%u", i );
            cmd[i].func   = GLCommand::GET_UNIFORM_LOCATION;
            cmd[i].arg[0] = progUid;
            cmd[i].arg[1] = uint64(tname[i]);
        }

        ExecGL( cmd, countof(cmd) );
        for( unsigned i=0; i!=countof(texunitLoc); ++i )
        {
            int loc = cmd[i].retval;
            
            texunitLoc[i] = (loc != -1)  ? loc  : InvalidIndex;
        }
    }

    texunitInited = false;
}


//------------------------------------------------------------------------------

unsigned
ProgGLES2::ConstBufferCount() const
{
    return countof(cbuf);
}


//------------------------------------------------------------------------------

Handle
ProgGLES2::InstanceConstBuffer( unsigned bufIndex )
{
    Handle  handle = InvalidHandle;

    DVASSERT(bufIndex < countof(cbuf));
//    DVASSERT(cbuf[bufIndex].location != InvalidIndex);
    
    if( bufIndex < countof(cbuf)  &&  cbuf[bufIndex].location != InvalidIndex )
    {
        handle = ConstBufGLES2Pool::Alloc();

        ConstBuf*   cb = ConstBufGLES2Pool::Get( handle );

        if( !cb->Construct( cbuf[bufIndex].location, cbuf[bufIndex].count ) )
        {
            ConstBufGLES2Pool::Free( handle );
            handle = InvalidHandle;
        }
    }

    return handle;
}


//------------------------------------------------------------------------------

void
ProgGLES2::SetToRHI() const
{

    if( !texunitInited )
    {
        for( unsigned i=0; i!=countof(texunitLoc); ++i )
        {
            if( texunitLoc[i] != -1 )
                GL_CALL(glUniform1i( texunitLoc[i], i ));
        }

        texunitInited = true;
    }
}


//------------------------------------------------------------------------------

bool
ProgGLES2::ConstBuf::Construct( unsigned loc, unsigned cnt )
{
    bool success = true;
    
    location    = loc;
    count       = cnt;
    data        = (float*)(::malloc( cnt*4*sizeof(float) ));
    inst        = nullptr;

    return success;
}


//------------------------------------------------------------------------------

void
ProgGLES2::ConstBuf::Destroy()
{
    if( data )
    {
        ::free( data );

        data     = nullptr;
        inst     = nullptr;
        location = -1;
        count    = 0;
    }
}


//------------------------------------------------------------------------------

unsigned    
ProgGLES2::ConstBuf::ConstCount() const
{
    return count;
}


//------------------------------------------------------------------------------

bool
ProgGLES2::ConstBuf::SetConst( unsigned const_i, unsigned const_count, const float* cdata )
{
    bool    success = false;

    if( const_i + const_count <= count )
    {
        memcpy( data + const_i*4, cdata, const_count*4*sizeof(float) );
        inst    = nullptr;
        success = true;
    }

    return success;
}


//------------------------------------------------------------------------------

bool
ProgGLES2::ConstBuf::SetConst( unsigned const_i, unsigned const_sub_i, const float* cdata, unsigned data_count )
{
    bool    success = false;

    if( const_i <= count  &&  const_sub_i < 4 )
    {
        memcpy( data + const_i*4 + const_sub_i, cdata, data_count*sizeof(float) );
        inst    = nullptr;
        success = true;
    }

    return success;
}


//------------------------------------------------------------------------------

const void*
ProgGLES2::ConstBuf::Instance() const
{
    if( !inst )
    {
//SCOPED_NAMED_TIMING("gl.cb-inst");
        inst = DefaultConstRingBuffer.Alloc( count*4*sizeof(float) );
        memcpy( inst, data, 4*count*sizeof(float) );
    }

    return inst;
}


//------------------------------------------------------------------------------

void
ProgGLES2::ConstBuf::SetToRHI( const void* instData ) const
{
//SCOPED_NAMED_TIMING("gl.cb-set");
    GL_CALL(glUniform4fv( location, count, (GLfloat*)instData ));
    StatSet::IncStat( stat_SET_CB, 1 );
}


//------------------------------------------------------------------------------

void
ProgGLES2::ConstBuf::InvalidateInstance()
{
    inst = nullptr;
}


//------------------------------------------------------------------------------

unsigned
ProgGLES2::ShaderUid() const
{
    return shader;
}


//------------------------------------------------------------------------------

unsigned        
ProgGLES2::TextureLocation( unsigned texunit_i ) const
{
    return texunitLoc[texunit_i];
}


//------------------------------------------------------------------------------

void
ProgGLES2::InvalidateAllConstBufferInstances()
{
    for( ConstBufGLES2Pool::Iterator b=ConstBufGLES2Pool::Begin(),b_end=ConstBufGLES2Pool::End(); b!=b_end; ++b )
    {
        b->InvalidateInstance();
    }
}



//------------------------------------------------------------------------------

static unsigned
gles2_ConstBuffer_ConstCount( Handle cb )
{
    const ProgGLES2::ConstBuf*  self = ConstBufGLES2Pool::Get( cb );

    return self->ConstCount();
}


//------------------------------------------------------------------------------

static bool
gles2_ConstBuffer_SetConst( Handle cb, unsigned const_i, unsigned const_count, const float* data )
{
    ProgGLES2::ConstBuf*  self = ConstBufGLES2Pool::Get( cb );

//Logger::Info( "  upd-cb %u", unsigned(RHI_HANDLE_INDEX(cb)) );
    return self->SetConst( const_i, const_count, data );
}


//------------------------------------------------------------------------------

static bool
gles2_ConstBuffer_SetConst1( Handle cb, unsigned const_i, unsigned const_sub_i, const float* data, unsigned dataCount )
{
    ProgGLES2::ConstBuf*  self = ConstBufGLES2Pool::Get( cb );

//Logger::Info( "  upd-cb %u", unsigned(RHI_HANDLE_INDEX(cb)) );
    return self->SetConst( const_i, const_sub_i, data, dataCount );
}


//------------------------------------------------------------------------------

static void
gles2_ConstBuffer_Delete( Handle cb )
{
    ProgGLES2::ConstBuf*  self = ConstBufGLES2Pool::Get( cb );

    self->Destroy();    
    ConstBufGLES2Pool::Free( cb );
}


//------------------------------------------------------------------------------

namespace ConstBufferGLES2
{

void
SetupDispatch( Dispatch* dispatch )
{
    dispatch->impl_ConstBuffer_SetConst     = &gles2_ConstBuffer_SetConst;
    dispatch->impl_ConstBuffer_SetConst1fv  = &gles2_ConstBuffer_SetConst1;
    dispatch->impl_ConstBuffer_ConstCount   = &gles2_ConstBuffer_ConstCount;
    dispatch->impl_ConstBuffer_Delete       = &gles2_ConstBuffer_Delete;
}

void
InitializeRingBuffer( uint32 size )
{
    DefaultConstRingBuffer.Initialize( size );
}

void
SetToRHI( const Handle cb, const void* instData )
{
    const ProgGLES2::ConstBuf*  self = ConstBufGLES2Pool::Get( cb );

//Logger::Info( "  set-cb %u  inst= %p", unsigned(RHI_HANDLE_INDEX(cb)), instData );
    self->SetToRHI( instData );
}

const void*
Instance( Handle cb )
{
    const ProgGLES2::ConstBuf*  self = ConstBufGLES2Pool::Get( cb );

    return self->Instance();
}

}


//==============================================================================
} // namespace rhi

