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
QueryBufferDX11_t
{
public:
    QueryBufferDX11_t()
        : curObjectIndex(DAVA::InvalidIndex)
        , bufferCompleted(false){};
    ~QueryBufferDX11_t() {};

    std::vector<std::pair<ID3D11Query*, uint32>> pendingQueries;
    std::vector<uint32> results;
    uint32 curObjectIndex;
    uint32 bufferCompleted : 1;
};

typedef ResourcePool<QueryBufferDX11_t, RESOURCE_QUERY_BUFFER, QueryBuffer::Descriptor, false> QueryBufferDX11Pool;
RHI_IMPL_POOL(QueryBufferDX11_t, RESOURCE_QUERY_BUFFER, QueryBuffer::Descriptor, false);

std::vector<ID3D11Query*> QueryDX11Pool;

//==============================================================================

static Handle
dx11_QueryBuffer_Create(uint32 maxObjectCount)
{
    Handle handle = QueryBufferDX11Pool::Alloc();
    QueryBufferDX11_t* buf = QueryBufferDX11Pool::Get(handle);
    DVASSERT(buf);

    buf->results.resize(maxObjectCount);
    memset(buf->results.data(), 0, sizeof(uint32) * buf->results.size());
    buf->pendingQueries.clear();
    buf->curObjectIndex = DAVA::InvalidIndex;
    buf->bufferCompleted = false;

    return handle;
}

static void
dx11_QueryBuffer_Delete(Handle handle)
{
    QueryBufferDX11_t* buf = QueryBufferDX11Pool::Get(handle);
    DVASSERT(buf);

    if (buf->pendingQueries.size())
    {
        for (size_t q = 0; q < buf->pendingQueries.size(); ++q)
            QueryDX11Pool.push_back(buf->pendingQueries[q].first);

        buf->pendingQueries.clear();
    }

    QueryBufferDX11Pool::Free(handle);
}

static void
dx11_QueryBuffer_Reset(Handle handle)
{
    QueryBufferDX11_t* buf = QueryBufferDX11Pool::Get(handle);
    DVASSERT(buf);

    memset(buf->results.data(), 0, sizeof(uint32) * buf->results.size());

    if (buf->pendingQueries.size())
    {
        for (size_t q = 0; q < buf->pendingQueries.size(); ++q)
            QueryDX11Pool.push_back(buf->pendingQueries[q].first);

        buf->pendingQueries.clear();
    }

    buf->curObjectIndex = DAVA::InvalidIndex;
    buf->bufferCompleted = false;
}

static void
dx11_Check_Query_Results(QueryBufferDX11_t* buf)
{
    int32 pendingCount = static_cast<int32>(buf->pendingQueries.size());
    uint64 val = 0;;
    for (int32 q = pendingCount - 1; q >= 0; --q)
    {
        ID3D11Query * iq = buf->pendingQueries[q].first;
        uint32 resultIndex = buf->pendingQueries[q].second;

        HRESULT hr = _D3D11_ImmediateContext->GetData(iq, &val, sizeof(uint64), D3D11_ASYNC_GETDATA_DONOTFLUSH);

        if (hr == S_OK)
        {
            buf->results[resultIndex] = static_cast<uint32>(val);
            QueryDX11Pool.push_back(buf->pendingQueries.back().first);

            buf->pendingQueries[q] = buf->pendingQueries.back();
            buf->pendingQueries.pop_back();
        }
    }
}

static bool
dx11_QueryBuffer_IsReady(Handle handle)
{
    bool ready = false;
    QueryBufferDX11_t* buf = QueryBufferDX11Pool::Get(handle);
    DVASSERT(buf);

    if (buf->bufferCompleted)
    {
        dx11_Check_Query_Results(buf);
        ready = (buf->pendingQueries.size() == 0);
    }

    return ready;
}

static bool
dx11_QueryBuffer_ObjectIsReady(Handle handle, uint32 objectIndex)
{
    bool ready = false;
    QueryBufferDX11_t* buf = QueryBufferDX11Pool::Get(handle);
    DVASSERT(buf);

    if (buf->bufferCompleted)
    {
        dx11_Check_Query_Results(buf);

        ready = true;
        for (size_t q = 0; q < buf->pendingQueries.size(); ++q)
        {
            if (buf->pendingQueries[q].second == objectIndex)
            {
                ready = false;
                break;
            }
        }
    }

    return ready;
}

static int
dx11_QueryBuffer_Value(Handle handle, uint32 objectIndex)
{
    QueryBufferDX11_t* buf = QueryBufferDX11Pool::Get(handle);
    DVASSERT(buf);

    dx11_Check_Query_Results(buf);

    if (objectIndex < buf->results.size())
    {
        return buf->results[objectIndex];
    }

    return 0;
}

namespace QueryBufferDX11
{
void SetupDispatch(Dispatch* dispatch)
{
    dispatch->impl_QueryBuffer_Create = &dx11_QueryBuffer_Create;
    dispatch->impl_QueryBuffer_Reset = &dx11_QueryBuffer_Reset;
    dispatch->impl_QueryBuffer_Delete = &dx11_QueryBuffer_Delete;
    dispatch->impl_QueryBuffer_IsReady = &dx11_QueryBuffer_IsReady;
    dispatch->impl_QueryBuffer_ObjectIsReady = &dx11_QueryBuffer_ObjectIsReady;
    dispatch->impl_QueryBuffer_Value = &dx11_QueryBuffer_Value;
}

void SetQueryIndex(Handle handle, uint32 objectIndex, ID3D11DeviceContext* context)
{
    QueryBufferDX11_t* buf = QueryBufferDX11Pool::Get(handle);
    DVASSERT(buf);

    if (buf->curObjectIndex != objectIndex)
    {
        if (buf->curObjectIndex != DAVA::InvalidIndex)
        {
            context->End(buf->pendingQueries.back().first);
            buf->curObjectIndex = DAVA::InvalidIndex;
        }

        if (objectIndex != DAVA::InvalidIndex)
        {
            ID3D11Query* iq = nullptr;
            if (QueryDX11Pool.size())
            {
                iq = QueryDX11Pool.back();
                QueryDX11Pool.pop_back();
            }
            else
            {
                D3D11_QUERY_DESC desc;

                desc.Query = D3D11_QUERY_OCCLUSION;
                desc.MiscFlags = 0;

                _D3D11_Device->CreateQuery(&desc, &iq);
            }

            if (iq)
            {
                context->Begin(iq);
                buf->pendingQueries.push_back(std::make_pair(iq, objectIndex));

                buf->curObjectIndex = objectIndex;
            }
        }
    }
}

void QueryComplete(Handle handle, ID3D11DeviceContext* context)
{
    QueryBufferDX11_t* buf = QueryBufferDX11Pool::Get(handle);
    DVASSERT(buf);

    if (buf->curObjectIndex != DAVA::InvalidIndex)
    {
        context->End(buf->pendingQueries.back().first);
        buf->curObjectIndex = DAVA::InvalidIndex;
    }

    buf->bufferCompleted = true;
}

bool QueryIsCompleted(Handle handle)
{
    QueryBufferDX11_t* buf = QueryBufferDX11Pool::Get(handle);
    DVASSERT(buf);

    return buf->bufferCompleted;
}

void ReleaseQueryPool()
{
    for (ID3D11Query* iq : QueryDX11Pool)
        iq->Release();

    QueryDX11Pool.clear();
}

}

//==============================================================================
} // namespace rhi
