#include "../Common/rhi_Private.h"
    #include "../Common/rhi_Pool.h"
    #include "rhi_DX11.h"

    #include "Debug/DVAssert.h"
    #include "Logger/Logger.h"
using DAVA::Logger;

    #include "_dx11.h"

namespace rhi
{
//==============================================================================

class
QueryBufferDX11_t
{
public:
    QueryBufferDX11_t();
    ~QueryBufferDX11_t();

    std::vector<ID3D11Query*> query;
};

typedef ResourcePool<QueryBufferDX11_t, RESOURCE_QUERY_BUFFER, QueryBuffer::Descriptor, false> QueryBufferDX11Pool;
RHI_IMPL_POOL(QueryBufferDX11_t, RESOURCE_QUERY_BUFFER, QueryBuffer::Descriptor, false);

//==============================================================================

QueryBufferDX11_t::QueryBufferDX11_t()
{
}

//------------------------------------------------------------------------------

QueryBufferDX11_t::~QueryBufferDX11_t()
{
}

static Handle
dx11_QueryBuffer_Create(uint32 maxObjectCount)
{
    Handle handle = QueryBufferDX11Pool::Alloc();
    QueryBufferDX11_t* buf = QueryBufferDX11Pool::Get(handle);

    if (buf)
    {
        buf->query.resize(maxObjectCount);
        memset(&(buf->query[0]), 0, sizeof(buf->query[0]) * buf->query.size());
    }

    return handle;
}

static void
dx11_QueryBuffer_Delete(Handle handle)
{
    QueryBufferDX11_t* buf = QueryBufferDX11Pool::Get(handle);

    if (buf)
    {
        for (std::vector<ID3D11Query *>::iterator q = buf->query.begin(), q_end = buf->query.end(); q != q_end; ++q)
        {
            if (*q)
                (*q)->Release();
        }

        buf->query.clear();
    }

    QueryBufferDX11Pool::Free(handle);
}

static void
dx11_QueryBuffer_Reset(Handle handle)
{
    QueryBufferDX11_t* buf = QueryBufferDX11Pool::Get(handle);

    if (buf)
    {
    }
}

static bool
dx11_QueryBuffer_IsReady(Handle handle, uint32 objectIndex)
{
    bool ready = false;
    QueryBufferDX11_t* buf = QueryBufferDX11Pool::Get(handle);

    if (buf && objectIndex < buf->query.size())
    {
        ID3D11Query* iq = buf->query[objectIndex];

        if (iq)
        {
            HRESULT hr = _D3D11_ImmediateContext->GetData(iq, NULL, 0, D3D11_ASYNC_GETDATA_DONOTFLUSH);
            CHECK_HR(hr)

            if (SUCCEEDED(hr))
            {
                ready = hr == S_OK;
            }
        }
    }

    return ready;
}

static int
dx11_QueryBuffer_Value(Handle handle, uint32 objectIndex)
{
    int value = 0;
    QueryBufferDX11_t* buf = QueryBufferDX11Pool::Get(handle);

    if (buf && objectIndex < buf->query.size())
    {
        ID3D11Query* iq = buf->query[objectIndex];

        if (iq)
        {
            UINT64 val;
            HRESULT hr = _D3D11_ImmediateContext->GetData(iq, &val, sizeof(UINT64), D3D11_ASYNC_GETDATA_DONOTFLUSH);
            CHECK_HR(hr)

            if (hr == S_OK)
            {
                value = int(val);
            }
        }
    }

    return value;
}

namespace QueryBufferDX11
{
void SetupDispatch(Dispatch* dispatch)
{
    dispatch->impl_QueryBuffer_Create = &dx11_QueryBuffer_Create;
    dispatch->impl_QueryBuffer_Reset = &dx11_QueryBuffer_Reset;
    dispatch->impl_QueryBuffer_Delete = &dx11_QueryBuffer_Delete;
    dispatch->impl_QueryBuffer_IsReady = &dx11_QueryBuffer_IsReady;
    dispatch->impl_QueryBuffer_Value = &dx11_QueryBuffer_Value;
}

void BeginQuery(Handle handle, uint32 objectIndex, ID3D11DeviceContext* context)
{
    QueryBufferDX11_t* buf = QueryBufferDX11Pool::Get(handle);

    if (buf && objectIndex < buf->query.size())
    {
        ID3D11Query* iq = buf->query[objectIndex];

        if (!iq)
        {
            D3D11_QUERY_DESC desc;

            desc.Query = D3D11_QUERY_OCCLUSION;
            desc.MiscFlags = 0;

            HRESULT hr = _D3D11_Device->CreateQuery(&desc, &iq);
            CHECK_HR(hr)

            if (SUCCEEDED(hr))
            {
                buf->query[objectIndex] = iq;
            }
            else
            {
                iq = nullptr;
            }
        }

        if (iq)
        {
            context->Begin(iq);
        }
    }
}

void EndQuery(Handle handle, uint32 objectIndex, ID3D11DeviceContext* context)
{
    QueryBufferDX11_t* buf = QueryBufferDX11Pool::Get(handle);

    if (buf && objectIndex < buf->query.size())
    {
        ID3D11Query* iq = buf->query[objectIndex];

        DVASSERT(iq);
        context->End(iq);
    }
}
}

//==============================================================================
} // namespace rhi
