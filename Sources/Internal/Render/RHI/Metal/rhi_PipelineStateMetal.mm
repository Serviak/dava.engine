
    #include "../Common/rhi_Private.h"
    #include "../Common/rhi_Impl.h"
    #include "../rhi_ShaderCache.h"
    #include "../Common/rhi_Pool.h"
    #include "rhi_RingBufferMetal.h"

    #include "FileSystem/Logger.h"
    using DAVA::Logger;
    #include "Debug/Profiler.h"

    #include "_metal.h"


namespace rhi
{

static inline unsigned
_VertexAttribIndex( VertexSemantics s, uint32 i )
{
    unsigned    attr_i   = InvalidIndex;
    
    switch( s )
    {
        case VS_POSITION    : attr_i = VATTR_POSITION; break;
        case VS_NORMAL      : attr_i = VATTR_NORMAL; break;
        case VS_TEXCOORD    :
        {
            switch( i )
            {
                case 0 : attr_i = VATTR_TEXCOORD_0; break;
                case 1 : attr_i = VATTR_TEXCOORD_1; break;
                case 2 : attr_i = VATTR_TEXCOORD_2; break;
                case 3 : attr_i = VATTR_TEXCOORD_3; break;
                case 4 : attr_i = VATTR_TEXCOORD_4; break;
                case 5 : attr_i = VATTR_TEXCOORD_5; break;
                case 6 : attr_i = VATTR_TEXCOORD_6; break;
                case 7 : attr_i = VATTR_TEXCOORD_7; break;
            }
        }   break;
        case VS_COLOR       :
        {
            switch( i )
            {
                case 0 : attr_i = VATTR_COLOR_0; break;
                case 1 : attr_i = VATTR_COLOR_1; break;
            }
        }   break;
        case VS_TANGENT     : attr_i = VATTR_TANGENT; break;
        case VS_BINORMAL    : attr_i = VATTR_BINORMAL; break;
        case VS_BLENDWEIGHT : attr_i = VATTR_BLENDWEIGHT; break;
        case VS_BLENDINDEX  : attr_i = VATTR_BLENDINDEX; break;
    }
    
    DVASSERT(attr_i != InvalidIndex);
    return attr_i;
}



class
PipelineStateMetal_t
{
public:
                        PipelineStateMetal_t()
                        {}

    class
    ConstBuf
    {
    public:
        
        enum ProgType { PROG_VERTEX, PROG_FRAGMENT };

                            ConstBuf()
                              : index(InvalidIndex),
                                count(0),
                                data(nullptr),
                                inst(nullptr),
                                inst_offset(0)
                            {}
                            ~ConstBuf()
                            {
                                ConstBuf::Destroy();
                            }
    
        bool                Construct( ProgType type, unsigned index, unsigned count );
        void                Destroy();

        unsigned            ConstCount() const;
        bool                SetConst( unsigned const_i, unsigned count, const float* cdata );
        bool                SetConst( unsigned const_i, unsigned const_sub_i, const float* cdata, unsigned data_count );
        
        void                SetToRHI( unsigned bufIndex, id<MTLRenderCommandEncoder> ce ) const;
        void                InvalidateInst();
    
    
    private:        

        ProgType            type;
        unsigned            index;
        unsigned            count;
        float*              data;
        mutable float*      inst;
        mutable unsigned    inst_offset;
    };

    struct
    VertexProg
    {
                        VertexProg()
                        {}
        void            GetBufferInfo( MTLRenderPipelineReflection* info );
        Handle          InstanceConstBuffer( unsigned buf_i );

        struct
        BufInfo
        {
            unsigned    index;
            unsigned    count;
            unsigned    used:1;
        };
        BufInfo         cbuf[MAX_CONST_BUFFER_COUNT];
    };

    struct
    FragmentProg
    {
                        FragmentProg()
                        {}
        void            GetBufferInfo( MTLRenderPipelineReflection* info );
        Handle          InstanceConstBuffer( unsigned buf_i );

        struct
        BufInfo
        {
            unsigned    index;
            unsigned    count;
            unsigned    used:1;
        };
        BufInfo         cbuf[MAX_CONST_BUFFER_COUNT];
    };

    VertexProg      vprog;
    FragmentProg    fprog;

    id<MTLRenderPipelineState>  state;

    VertexLayout                    layout;
    MTLRenderPipelineDescriptor*    desc;
    struct
    state_t
    {
        uint32                      layoutUID;
        id<MTLRenderPipelineState>  state;
    };
    std::vector<state_t>            altState;
};

typedef Pool<PipelineStateMetal_t,RESOURCE_PIPELINE_STATE>          PipelineStateMetalPool;
typedef Pool<PipelineStateMetal_t::ConstBuf,RESOURCE_CONST_BUFFER>  ConstBufMetalPool;

RHI_IMPL_POOL(PipelineStateMetal_t,RESOURCE_PIPELINE_STATE);
RHI_IMPL_POOL(PipelineStateMetal_t::ConstBuf,RESOURCE_CONST_BUFFER);


static RingBufferMetal  DefaultConstRingBuffer;
static RingBufferMetal  VertexConstRingBuffer;
static RingBufferMetal  FragmentConstRingBuffer;



//------------------------------------------------------------------------------

void
PipelineStateMetal_t::FragmentProg::GetBufferInfo( MTLRenderPipelineReflection* info )
{
    for( unsigned i=0; i!=countof(cbuf); ++i )
    {
        cbuf[i].index = i;
        cbuf[i].count = 0;
        cbuf[i].used  = false;
    
        for( MTLArgument* arg in info.fragmentArguments )
        {
            if(     arg.active  
                &&  arg.type == MTLArgumentTypeBuffer 
                &&  arg.index == i
              )
            {
                MTLStructType*  str = arg.bufferStructType;
                
                for( MTLStructMember* member in str.members )
                {
                    if( member.dataType == MTLDataTypeArray )
                    {
                        MTLArrayType*   arr = member.arrayType;
                        
                        cbuf[i].index = i;
                        cbuf[i].count = arr.arrayLength;
                        cbuf[i].used  = true;
                        break;
                    }
                }
                
                break;
            }
        }
    }
}


//------------------------------------------------------------------------------

Handle
PipelineStateMetal_t::FragmentProg::InstanceConstBuffer( unsigned bufIndex )
{
    Handle  handle = InvalidHandle;

    DVASSERT(bufIndex < countof(cbuf));
//    DVASSERT(cbuf[bufIndex].location != InvalidIndex);
    
    if( bufIndex < countof(cbuf)  &&  cbuf[bufIndex].index != InvalidIndex )
    {
        handle = ConstBufMetalPool::Alloc();

        ConstBuf*   cb = ConstBufMetalPool::Get( handle );

        if( !cb->Construct( ConstBuf::PROG_FRAGMENT, cbuf[bufIndex].index, cbuf[bufIndex].count ) )
        {
            ConstBufMetalPool::Free( handle );
            handle = InvalidHandle;
        }
    }

    return handle;
}


//------------------------------------------------------------------------------

void
PipelineStateMetal_t::VertexProg::GetBufferInfo( MTLRenderPipelineReflection* info )
{
    for( unsigned i=0; i!=countof(cbuf); ++i )
    {
        cbuf[i].index = i;
        cbuf[i].count = 0;
        cbuf[i].used  = false;

        for( MTLArgument* arg in info.vertexArguments )
        {
//            const char* name = arg.name.UTF8String;
            if(     arg.active  
                &&  arg.type == MTLArgumentTypeBuffer 
                &&  arg.index == 1+i // CRAP: vprog-buf#0 assumed to be vdata
              )
            {
                MTLStructType*  str = arg.bufferStructType;
                
                for( MTLStructMember* member in str.members )
                {
                    if( member.dataType == MTLDataTypeArray )
                    {
                        MTLArrayType*   arr = member.arrayType;
                        
                        cbuf[i].index = i;
                        cbuf[i].count = arr.arrayLength;
                        cbuf[i].used  = true;
                        break;
                    }
                }
                break;
            }
        }
    }
}


//------------------------------------------------------------------------------

Handle
PipelineStateMetal_t::VertexProg::InstanceConstBuffer( unsigned bufIndex )
{   
    Handle  handle = InvalidHandle;

    DVASSERT(bufIndex < countof(cbuf));
//    DVASSERT(cbuf[bufIndex].location != InvalidIndex);
    
    if( bufIndex < countof(cbuf)  &&  cbuf[bufIndex].index != InvalidIndex )
    {
        handle = ConstBufMetalPool::Alloc();

        ConstBuf*   cb = ConstBufMetalPool::Get( handle );

        if( !cb->Construct( ConstBuf::PROG_VERTEX, cbuf[bufIndex].index, cbuf[bufIndex].count ) )
        {
            ConstBufMetalPool::Free( handle );
            handle = InvalidHandle;
        }
    }

    return handle;
}

    
//------------------------------------------------------------------------------

bool
PipelineStateMetal_t::ConstBuf::Construct( PipelineStateMetal_t::ConstBuf::ProgType ptype, unsigned buf_i, unsigned cnt )
{
    type        = ptype;
    index       = buf_i;
    count       = cnt;
    data        = (float*)::malloc( cnt*4*sizeof(float) );
    inst        = nullptr;
    inst_offset = 0;

    return true;
}


//------------------------------------------------------------------------------

void
PipelineStateMetal_t::ConstBuf::Destroy()
{
    if( data )
    {
        ::free( data );
        data = nullptr;
    }
    
    index       = InvalidIndex;
    count       = 0;
    inst        = 0;
    inst_offset = 0;
}


//------------------------------------------------------------------------------

unsigned
PipelineStateMetal_t::ConstBuf::ConstCount() const
{
    return count;
}


//------------------------------------------------------------------------------

bool
PipelineStateMetal_t::ConstBuf::SetConst( unsigned const_i, unsigned const_count, const float* cdata )
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
PipelineStateMetal_t::ConstBuf::SetConst( unsigned const_i, unsigned const_sub_i, const float* cdata, unsigned data_count )
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

void
PipelineStateMetal_t::ConstBuf::SetToRHI( unsigned bufIndex, id<MTLRenderCommandEncoder> ce ) const
{
    id<MTLBuffer>   buf = DefaultConstRingBuffer.BufferUID();
    
    if( !inst )
    {
        inst = DefaultConstRingBuffer.Alloc( count*4*sizeof(float), &inst_offset );
//        inst = (type == PROG_VERTEX)
//               ? VertexConstRingBuffer.Alloc( count*4*sizeof(float), &inst_offset )
//               : FragmentConstRingBuffer.Alloc( count*4*sizeof(float), &inst_offset );

        memcpy( inst, data, count*4*sizeof(float) );
    }

    if( type == PROG_VERTEX )
        [ce setVertexBuffer:buf offset:inst_offset atIndex:1+bufIndex]; // CRAP: vprog-buf#0 assumed to be vdata
    else
        [ce setFragmentBuffer:buf offset:inst_offset atIndex:bufIndex];

//    if( type == PROG_VERTEX )
//        [ce setVertexBuffer:VertexConstRingBuffer.BufferUID() offset:inst_offset atIndex:1+bufIndex]; // CRAP: vprog-buf#0 assumed to be vdata
//    else
//        [ce setFragmentBuffer:FragmentConstRingBuffer.BufferUID() offset:inst_offset atIndex:bufIndex];
}


//------------------------------------------------------------------------------

void 
PipelineStateMetal_t::ConstBuf::InvalidateInst()
{
    inst = nullptr;
}

    
//==============================================================================

static Handle
metal_PipelineState_Create( const PipelineState::Descriptor& desc )
{
    Handle                      handle      = PipelineStateMetalPool::Alloc();;
    PipelineStateMetal_t*       ps          = PipelineStateMetalPool::Get( handle );
    static std::vector<uint8>   vprog_bin;
    static std::vector<uint8>   fprog_bin;

    rhi::ShaderCache::GetProg( desc.vprogUid, &vprog_bin );
    rhi::ShaderCache::GetProg( desc.fprogUid, &fprog_bin );


    // compile vprog

    NSString*           vp_src  = [NSString stringWithUTF8String:(const char*)(&vprog_bin[0])];
    MTLCompileOptions*  vp_opt  = [MTLCompileOptions alloc];
    NSError*            vp_err  = nil;
    id<MTLLibrary>      vp_lib  = [_Metal_Device newLibraryWithSource:vp_src options:vp_opt error:&vp_err];
    id<MTLFunction>     vp_func = [vp_lib newFunctionWithName:@"vp_main"];

    if( vp_err == nil )
    {
        if( vp_func == nil )
        {
            Logger::Error( "FAILED to get vprog \"%s\" function", desc.fprogUid.c_str() );
        }
    }
    else
    {
        Logger::Error( "FAILED to compile vprog \"%s\" :\n%s", desc.vprogUid.c_str(), vp_err.localizedDescription.UTF8String );
    }

    
    // compile fprog

    NSString*           fp_src  = [NSString stringWithUTF8String:(const char*)(&fprog_bin[0])];
    MTLCompileOptions*  fp_opt  = [MTLCompileOptions alloc];
    NSError*            fp_err  = nil;
    id<MTLLibrary>      fp_lib  = [_Metal_Device newLibraryWithSource:fp_src options:fp_opt error:&fp_err];
    id<MTLFunction>     fp_func = nil;
    
    if( fp_err == nil )
    {
        fp_func = [fp_lib newFunctionWithName:@"fp_main"];
        
        if( fp_func == nil )
        {
            Logger::Error( "FAILED to get fprog \"%s\" function", desc.fprogUid.c_str() );
        }
    }
    else
    {
        Logger::Error( "FAILED to compile fprog \"%s\" :\n%s", desc.fprogUid.c_str(), fp_err.localizedDescription.UTF8String );
    }


    // create render-state
    
    if( vp_func != nil  &&  fp_func != nil )
    {
        MTLRenderPipelineDescriptor*    rp_desc = [MTLRenderPipelineDescriptor new];
        MTLRenderPipelineReflection*    ps_info = nil;
        NSError*                        rs_err  = nil;
        
        rp_desc.depthAttachmentPixelFormat          = MTLPixelFormatDepth32Float;
        rp_desc.colorAttachments[0].pixelFormat     = MTLPixelFormatBGRA8Unorm;
        rp_desc.colorAttachments[0].blendingEnabled = desc.blending.rtBlend[0].blendEnabled;
        rp_desc.sampleCount                         = 1;
        rp_desc.vertexFunction                      = vp_func;
        rp_desc.fragmentFunction                    = fp_func;
        
        if( desc.blending.rtBlend[0].blendEnabled )
        {
            switch( desc.blending.rtBlend[0].colorSrc )
            {
                case BLENDOP_ZERO :
                    rp_desc.colorAttachments[0].sourceRGBBlendFactor = MTLBlendFactorZero;
                    rp_desc.colorAttachments[0].sourceAlphaBlendFactor = MTLBlendFactorZero;
                    break;
                
                case BLENDOP_ONE :
                    rp_desc.colorAttachments[0].sourceRGBBlendFactor = MTLBlendFactorOne;
                    rp_desc.colorAttachments[0].sourceAlphaBlendFactor = MTLBlendFactorOne;
                    break;
                
                case BLENDOP_SRC_ALPHA :
                    rp_desc.colorAttachments[0].sourceRGBBlendFactor = MTLBlendFactorSourceAlpha;
                    rp_desc.colorAttachments[0].sourceAlphaBlendFactor = MTLBlendFactorSourceAlpha;
                    break;
                
                case BLENDOP_INV_SRC_ALPHA :
                    rp_desc.colorAttachments[0].sourceRGBBlendFactor = MTLBlendFactorOneMinusSourceAlpha;
                    rp_desc.colorAttachments[0].sourceAlphaBlendFactor = MTLBlendFactorOneMinusSourceAlpha;
                    break;
                
                case BLENDOP_SRC_COLOR :
                    rp_desc.colorAttachments[0].sourceRGBBlendFactor = MTLBlendFactorSourceColor;
                    rp_desc.colorAttachments[0].sourceAlphaBlendFactor = MTLBlendFactorSourceColor;
                    break;
                
                case BLENDOP_DST_COLOR :
                    rp_desc.colorAttachments[0].sourceRGBBlendFactor = MTLBlendFactorDestinationColor;
                    rp_desc.colorAttachments[0].sourceAlphaBlendFactor = MTLBlendFactorDestinationColor;
                    break;
            }

            switch( desc.blending.rtBlend[0].colorDst )
            {
                case BLENDOP_ZERO :
                    rp_desc.colorAttachments[0].destinationRGBBlendFactor = MTLBlendFactorZero;
                    rp_desc.colorAttachments[0].destinationAlphaBlendFactor = MTLBlendFactorZero;
                    break;
                    
                case BLENDOP_ONE :
                    rp_desc.colorAttachments[0].destinationRGBBlendFactor = MTLBlendFactorOne;
                    rp_desc.colorAttachments[0].destinationAlphaBlendFactor = MTLBlendFactorOne;
                    break;
                    
                case BLENDOP_SRC_ALPHA :
                    rp_desc.colorAttachments[0].destinationRGBBlendFactor = MTLBlendFactorSourceAlpha;
                    rp_desc.colorAttachments[0].destinationAlphaBlendFactor = MTLBlendFactorSourceAlpha;
                    break;
                    
                case BLENDOP_INV_SRC_ALPHA :
                    rp_desc.colorAttachments[0].destinationRGBBlendFactor = MTLBlendFactorOneMinusSourceAlpha;
                    rp_desc.colorAttachments[0].destinationAlphaBlendFactor = MTLBlendFactorOneMinusSourceAlpha;
                    break;
                
                case BLENDOP_SRC_COLOR :
                    rp_desc.colorAttachments[0].destinationRGBBlendFactor = MTLBlendFactorSourceColor;
                    rp_desc.colorAttachments[0].destinationAlphaBlendFactor = MTLBlendFactorSourceColor;
                    break;
                
                case BLENDOP_DST_COLOR :
                    rp_desc.colorAttachments[0].destinationRGBBlendFactor = MTLBlendFactorDestinationColor;
                    rp_desc.colorAttachments[0].destinationAlphaBlendFactor = MTLBlendFactorDestinationColor;
                    break;
            }
        }

        rp_desc.colorAttachments[0].writeMask = MTLColorWriteMaskNone;
        
        if( desc.blending.rtBlend[0].writeMask & COLORMASK_R )
            rp_desc.colorAttachments[0].writeMask = MTLColorWriteMaskRed;
        if( desc.blending.rtBlend[0].writeMask & COLORMASK_G )
            rp_desc.colorAttachments[0].writeMask = MTLColorWriteMaskGreen;
        if( desc.blending.rtBlend[0].writeMask & COLORMASK_B )
            rp_desc.colorAttachments[0].writeMask = MTLColorWriteMaskBlue;
        if( desc.blending.rtBlend[0].writeMask & COLORMASK_A )
            rp_desc.colorAttachments[0].writeMask = MTLColorWriteMaskAlpha;

        for( unsigned i=0; i!=VATTR_COUNT; ++i )
        {
            rp_desc.vertexDescriptor.attributes[i].bufferIndex = 0;
            rp_desc.vertexDescriptor.attributes[i].offset      = 0;
            rp_desc.vertexDescriptor.attributes[i].format      = MTLVertexFormatFloat;
        }

        for( unsigned i=0; i!=desc.vertexLayout.ElementCount(); ++i )
        {
            unsigned        attr_i   = _VertexAttribIndex( desc.vertexLayout.ElementSemantics(i), desc.vertexLayout.ElementSemanticsIndex(i) );
            MTLVertexFormat fmt      = MTLVertexFormatInvalid;
            
            switch( desc.vertexLayout.ElementDataType(i) )
            {
                case VDT_FLOAT :
                {
                    switch( desc.vertexLayout.ElementDataCount(i) )
                    {
                        case 1 : fmt = MTLVertexFormatFloat; break;
                        case 2 : fmt = MTLVertexFormatFloat2; break;
                        case 3 : fmt = MTLVertexFormatFloat3; break;
                        case 4 : fmt = MTLVertexFormatFloat4; break;
                    }
                }   break;
                    
                case VDT_UINT8 :
                case VDT_UINT8N :
                {
                    switch( desc.vertexLayout.ElementDataCount(i) )
                    {
                            //                                    case 1 : fmt = MTLVertexFormatUCharNormalized; break;
                        case 2 : fmt = MTLVertexFormatUChar2; break;
                        case 3 : fmt = MTLVertexFormatUChar3; break;
                        case 4 : fmt = MTLVertexFormatUChar4; break;
                    }
                }   break;
            }
            
            rp_desc.vertexDescriptor.attributes[attr_i].bufferIndex = 0 ;
            rp_desc.vertexDescriptor.attributes[attr_i].offset      = desc.vertexLayout.ElementOffset(i);
            rp_desc.vertexDescriptor.attributes[attr_i].format      = fmt;
        }
        
        rp_desc.vertexDescriptor.layouts[0].stepFunction   = MTLVertexStepFunctionPerVertex;
        rp_desc.vertexDescriptor.layouts[0].stepRate       = 1;
        rp_desc.vertexDescriptor.layouts[0].stride         = desc.vertexLayout.Stride();
        
        ps->state = [_Metal_Device newRenderPipelineStateWithDescriptor:rp_desc options:MTLPipelineOptionBufferTypeInfo reflection:&ps_info error:&rs_err];
        
        if( rs_err == nil )
        {
            ps->vprog.GetBufferInfo( ps_info );
            ps->fprog.GetBufferInfo( ps_info );
            
            ps->desc    = rp_desc;
            ps->layout  = desc.vertexLayout;
        }
        else
        {
            Logger::Error( "FAILED create pipeline-state:\n%s", rs_err.localizedDescription.UTF8String );
            DVASSERT(false);
        }
    }
    else
    {
        handle = InvalidHandle;
    }

    return handle;
} 


//------------------------------------------------------------------------------

static void
metal_PipelineState_Delete( Handle ps )
{
}


//------------------------------------------------------------------------------

static Handle
metal_PipelineState_CreateVertexConstBuffer( Handle ps, unsigned bufIndex )
{
    PipelineStateMetal_t* psm = PipelineStateMetalPool::Get( ps );
    
    return psm->vprog.InstanceConstBuffer( bufIndex );
}


//------------------------------------------------------------------------------

static Handle
metal_PipelineState_CreateFragmentConstBuffer( Handle ps, unsigned bufIndex )
{
    PipelineStateMetal_t* psm = PipelineStateMetalPool::Get( ps );
    
    return psm->fprog.InstanceConstBuffer( bufIndex );
}



static bool
metal_ConstBuffer_SetConst( Handle cb, uint32 constIndex, uint32 constCount, const float* data )
{
    PipelineStateMetal_t::ConstBuf* buf = ConstBufMetalPool::Get( cb );

    return buf->SetConst( constIndex, constCount, data );
}

static bool
metal_ConstBuffer_SetConst1fv( Handle cb, uint32 constIndex, uint32 constSubIndex, const float* data, uint32 dataCount )
{
    PipelineStateMetal_t::ConstBuf* buf = ConstBufMetalPool::Get( cb );

    return buf->SetConst( constIndex, constSubIndex, data, dataCount );
}

static void
metal_ConstBuffer_Delete( Handle cb )
{
    PipelineStateMetal_t::ConstBuf* buf = ConstBufMetalPool::Get( cb );
    
    buf->Destroy();
    ConstBufMetalPool::Free( cb );
}


namespace ConstBufferMetal
{

void
SetupDispatch( Dispatch* dispatch )
{
    dispatch->impl_ConstBuffer_SetConst     = &metal_ConstBuffer_SetConst;
    dispatch->impl_ConstBuffer_SetConst1fv  = &metal_ConstBuffer_SetConst1fv;
    dispatch->impl_ConstBuffer_ConstCount   = nullptr;//&metal_ConstBuffer_ConstCount;
    dispatch->impl_ConstBuffer_Delete       = &metal_ConstBuffer_Delete;
}

}



namespace PipelineStateMetal
{

void
SetupDispatch( Dispatch* dispatch )
{
    dispatch->impl_PipelineState_Create                     = &metal_PipelineState_Create;
    dispatch->impl_PipelineState_Delete                     = &metal_PipelineState_Delete;
    dispatch->impl_PipelineState_CreateVertexConstBuffer    = &metal_PipelineState_CreateVertexConstBuffer;
    dispatch->impl_PipelineState_CreateFragmentConstBuffer  = &metal_PipelineState_CreateFragmentConstBuffer;
}

void
SetToRHI( Handle ps, uint32 layoutUID, id<MTLRenderCommandEncoder> ce )
{
    PipelineStateMetal_t* psm = PipelineStateMetalPool::Get( ps );

    DVASSERT(psm);

    if( layoutUID == VertexLayout::InvalidUID )
    {
        [ce setRenderPipelineState:psm->state];
    }
    else
    {
        bool        do_add  = true;
        unsigned    si      = InvalidIndex;

        for( unsigned i=0; i!=psm->altState.size(); ++i )
        {
            if( psm->altState[i].layoutUID == layoutUID )
            {
                si     = i;
                do_add = false;
                break;
            }
        }

        if( do_add )
        {
            PipelineStateMetal_t::state_t   state;
            const VertexLayout*             layout  = VertexLayout::Get( layoutUID );
            MTLRenderPipelineDescriptor*    rp_desc = [MTLRenderPipelineDescriptor new];
            MTLRenderPipelineReflection*    ps_info = nil;
            NSError*                        rs_err  = nil;
            
            rp_desc.depthAttachmentPixelFormat  = MTLPixelFormatDepth32Float;
            rp_desc.colorAttachments[0]         = psm->desc.colorAttachments[0];
            rp_desc.sampleCount                 = 1;
            rp_desc.vertexFunction              = psm->desc.vertexFunction;
            rp_desc.fragmentFunction            = psm->desc.fragmentFunction;

            for( unsigned i=0; i!=VATTR_COUNT; ++i )
            {
                rp_desc.vertexDescriptor.attributes[i].bufferIndex = 0;
                rp_desc.vertexDescriptor.attributes[i].offset      = 0;
                rp_desc.vertexDescriptor.attributes[i].format      = MTLVertexFormatFloat;
            }
            
            for( unsigned i=0; i!=psm->layout.ElementCount(); ++i )
            {
                unsigned    attr_i   = _VertexAttribIndex( psm->layout.ElementSemantics(i), psm->layout.ElementSemanticsIndex(i) );
                bool        attr_set = false;
                
                for( unsigned j=0; j!=layout->ElementCount(); ++j )
                {
                    if( layout->ElementSemantics(j) == psm->layout.ElementSemantics(i) )
                    {
                        MTLVertexFormat fmt = MTLVertexFormatInvalid;
                        
                        switch( psm->layout.ElementDataType(i) )
                        {
                            case VDT_FLOAT :
                            {
                                switch( psm->layout.ElementDataCount(i) )
                                {
                                    case 1 : fmt = MTLVertexFormatFloat; break;
                                    case 2 : fmt = MTLVertexFormatFloat2; break;
                                    case 3 : fmt = MTLVertexFormatFloat3; break;
                                    case 4 : fmt = MTLVertexFormatFloat4; break;
                                }
                            }   break;
                            
                            case VDT_UINT8 :
                            case VDT_UINT8N :
                            {
                                switch( psm->layout.ElementDataCount(i) )
                                {
//                                    case 1 : fmt = MTLVertexFormatUCharNormalized; break;
                                    case 2 : fmt = MTLVertexFormatUChar2Normalized; break;
                                    case 3 : fmt = MTLVertexFormatUChar3Normalized; break;
                                    case 4 : fmt = MTLVertexFormatUChar4Normalized; break;
                                }
                            }   break;
                        }

                        rp_desc.vertexDescriptor.attributes[attr_i].bufferIndex = 0 ;
                        rp_desc.vertexDescriptor.attributes[attr_i].offset      = layout->ElementOffset(j);
                        rp_desc.vertexDescriptor.attributes[attr_i].format      = fmt;
                        
                        attr_set = true;
                        break;
                    }
                }
                DVASSERT(attr_set);
            }
            
            rp_desc.vertexDescriptor.layouts[0].stepFunction   = MTLVertexStepFunctionPerVertex;
            rp_desc.vertexDescriptor.layouts[0].stepRate       = 1;
            rp_desc.vertexDescriptor.layouts[0].stride         = layout->Stride();
            
            state.layoutUID = layoutUID;
            state.state     = [_Metal_Device newRenderPipelineStateWithDescriptor:rp_desc options:MTLPipelineOptionNone reflection:&ps_info error:&rs_err];
            
            si = psm->altState.size();
            psm->altState.push_back( state );
        }
    
        DVASSERT(si != InvalidIndex);
        [ce setRenderPipelineState:psm->altState[si].state];
    }
}

} // namespace PipelineStateMetal


namespace ConstBufferMetal
{

void
InitializeRingBuffer( uint32 size )
{
    DefaultConstRingBuffer.Initialize( size );
//    VertexConstRingBuffer.Initialize( size );
//    FragmentConstRingBuffer.Initialize( size );
}


void
SetToRHI( Handle buf, unsigned bufIndex, id<MTLRenderCommandEncoder> ce )
{
    PipelineStateMetal_t::ConstBuf* cbuf = ConstBufMetalPool::Get( buf );

    cbuf->SetToRHI( bufIndex, ce );
}

void    
InvalidateAllInstances()
{
    for( ConstBufMetalPool::Iterator b=ConstBufMetalPool::Begin(),b_end=ConstBufMetalPool::End(); b!=b_end; ++b )
    {
        b->InvalidateInst();
    }
}

}




} // namespace rhi
